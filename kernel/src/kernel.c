#include "kernel.h"

void sigint_handler(int signum) {
    // Acciones a realizar al recibir la señal SIGINT
    // Hay mas cosas para hacer pero estas son algunas por ahora
    printf("Programa finalizado con SIGINT\n");
    cerrar_programa(logger, config);
    string_array_destroy(config_kernel->RECURSOS);
    string_array_destroy(config_kernel->INSTANCIAS_RECURSOS);
    free(config_kernel);
    list_destroy(lista_pids);
    list_destroy_and_destroy_elements(lista_global_procesos, free);
    list_destroy_and_destroy_elements(tabla_global_archivos_abiertos, free); //VER ESTO -> POSIBLE LEAK

    
    exit(0);
}


int main(int argc, char** argv){
    signal(SIGINT, sigint_handler);

    logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
    // t_config *config = config_create("../tests/DEADLOCK/kernel.config"); //cambiar BASE por el nombre de la prueba segun corresponda. Hay que ordenar esto
    if (argc==1){
        config = config_create("kernel.config");
    }
    else if (argc==2) {
        config = config_create(argv[1]); // Un ejemplo seria ./kernel "../tests/BASE/kernel.config"
    }
    else{
        log_error(logger, "Cantidad de parametros incorrecta. Debería ser ./kernel PATH_CONFIG \n"); 
        return EXIT_FAILURE;
    }

    cargar_config_kernel(config);
    
    //Aca comienza a correr el clock global
    temporal = temporal_create();
    
    
    lista_global_procesos = list_create();

    inicializar_recursos();
    
    
    //mostrar_lista(recursos);

    inicializar_diccionarios();
    inicializar_semaforos();
    inicializar_colas();

    
    /* ------------------------- INICIAR SERVIDOR -------------------------*/
    int kernel_fd = preparar_servidor("KERNEL", config, logger);


    printf("FALOPA1");

    /* ------------------------- CONEXION CON CPU -------------------------*/
    server_cpu = conectar_con(CPU, config, logger);


    printf("FALOPA2");

    /* -------------------------CONEXION CON FILESYSTEM --------------------*/
    server_fs = conectar_con(FILESYSTEM, config, logger);


    printf("FALOPA3");

    /* ------------------------- CONEXION CON MEMORIA -----------------------*/
    server_memoria = conectar_con(MEMORIA, config, logger);

    RECV_INT(server_memoria, cant_segmentos);

    log_debug(logger, "Recibi la cantidad de segmentos de memoria");

    printf("FALOPA4");

    /* ------------------------- PLANIFICADOR LARGO PLAZO -------------------------*/
    pthread_t hilo_planificador_largo;
	pthread_create(&hilo_planificador_largo, NULL, (void*)planificador_largo_plazo, NULL);
	pthread_detach(hilo_planificador_largo);


    /* ------------------------- PLANIFICADOR CORTITO PLAZO -------------------------*/
    pthread_t hilo_planificador_corto;
	pthread_create(&hilo_planificador_corto, NULL, (void*)planificador_corto_plazo, NULL);
	pthread_detach(hilo_planificador_corto);


    /* ------------------------- PROCESAR CPU -------------------------*/
    pthread_t hilo_kernel_cpu;
    pthread_create(&hilo_kernel_cpu, NULL, (void*)procesar_cpu_kernel, NULL);
    pthread_detach(hilo_kernel_cpu);


    /* ------------------------- PROCESAR CONSOLAS -------------------------*/
    // Hilo main espera clientes, por cada cliente que se conecta crea un hilo extra para procesar la conexión del mismo
    while (1)
    {
        int consola_fd = esperar_cliente(kernel_fd, logger, "Kernel");
        
        pthread_t hilo;
        pthread_create(&hilo, NULL, (void *)procesar_consola, (void *) (intptr_t) consola_fd);
        pthread_detach(hilo);
    }

    return 0;
}