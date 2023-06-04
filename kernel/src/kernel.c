#include "kernel.h"

int main(int argc, char** argv){
    logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    // t_config *config = config_create("../tests/DEADLOCK/kernel.config"); //cambiar BASE por el nombre de la prueba segun corresponda. Hay que ordenar esto
    t_config *config = config_create("kernel.config");
    cargar_config_kernel(config);
  

    inicializar_recursos();
    
    
    //mostrar_lista(recursos);

    inicializar_diccionarios();
    inicializar_semaforos();
    inicializar_colas();

    
    /* ------------------------- INICIAR SERVIDOR -------------------------*/
    int server_fd = preparar_servidor("KERNEL", config, logger);


    /* ------------------------- CONEXION CON CPU -------------------------*/
    int conexion_cpu = conectar_con(CPU, config, logger);


    /* -------------------------CONEXION CON FILESYSTEM --------------------*/
    //int conexion_fs = conectar_con(FILESYSTEM, config, logger);


    /* ------------------------- CONEXION CON MEMORIA -----------------------*/
    //int conexion_mem = conectar_con(MEMORIA, config, logger);




    /* ------------------------- PLANIFICADOR LARGO PLAZO -------------------------*/
    pthread_t hilo_planificador_largo;
	pthread_create(&hilo_planificador_largo, NULL, (void*)planificador_largo_plazo, (void*) logger);
	pthread_detach(hilo_planificador_largo);


    /* ------------------------- PLANIFICADOR CORTITO PLAZO -------------------------*/
    pthread_t hilo_planificador_corto;
	pthread_create(&hilo_planificador_corto, NULL, (void*)planificador_corto_plazo, (void *) (intptr_t) conexion_cpu);
	pthread_detach(hilo_planificador_corto);


    /* ------------------------- PROCESAR CPU -------------------------*/
    pthread_t hilo_kernel_cpu;
    pthread_create(&hilo_kernel_cpu, NULL, (void*)procesar_cpu, (void*) (intptr_t) conexion_cpu);
    pthread_detach(hilo_kernel_cpu);



    /* ------------------------- PROCESAR CONSOLAS -------------------------*/
    // Hilo main espera clientes, por cada cliente que se conecta crea un hilo extra para procesar la conexión del mismo
    while (1)
    {
        int consola_fd = esperar_cliente(server_fd, logger, "Kernel");
        
        pthread_t hilo;
        pthread_create(&hilo, NULL, (void *)procesar_consola, (void *) (intptr_t) consola_fd);
        pthread_detach(hilo);
    }

    cerrar_programa(logger, config);
    free(config_kernel);
    list_destroy_and_destroy_elements(lista_pids, free);

    return 0;
}