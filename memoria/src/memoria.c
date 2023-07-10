#include "memoria.h"

int main(int argc, char** argv){
    logger = log_create("memoria.log", "MEMORIA", true, LOG_LEVEL_DEBUG);

    if (argc==1){
        config = config_create("memoria.config");
    }
    else if (argc==2) {
        config = config_create(argv[1]); // Un ejemplo seria ../tests/BASE/memoria.config
    }
    else{
        log_error(logger, "Cantidad de parametros incorrecta. Debería ser ./memoria PATH_CONFIG \n"); 
        return EXIT_FAILURE;
    }

    cargar_config_memoria(config);
    
    // SERVER -> CPU, Kernel, FileSystem
    int server_fd = preparar_servidor("MEMORIA", config, logger);

    //cliente_filesystem = esperar_cliente(server_fd, logger, "Memoria");
    cliente_cpu = esperar_cliente(server_fd, logger, "MEMORIA");
    cliente_kernel = esperar_cliente(server_fd, logger, "MEMORIA");
    
    // Recibimos el handshake
    //int handshake_cpu;
    //RECV_INT(cliente_cpu, handshake_cpu);

    // Respuesta handshake
    //if(handshake_cpu == 1) {
    //    SEND_INT(cliente_cpu, 2); 
   // }

    
    
    // Recibimos el tamanio maximo de segmento indicado por CPU
    RECV_INT(cliente_cpu, tamanio_max_segmento_cpu);

    // Mandamos a Kernel la cantidad de segmentos
    SEND_INT(cliente_kernel, config_memoria.CANT_SEGMENTOS);

    log_info(logger, "Tamanio maximo de segmento: %d", tamanio_max_segmento_cpu);

    inicializar_estructuras_administrativas();
    
    pthread_t hilo_procesar_kernel_memoria;
    pthread_create(&hilo_procesar_kernel_memoria, NULL, (void*)procesar_kernel_memoria, NULL);
    pthread_detach(hilo_procesar_kernel_memoria);
    
    pthread_t hilo_procesar_cpu_memoria;
    pthread_create(&hilo_procesar_cpu_memoria, NULL, (void*)procesar_cpu_memoria, NULL);
    pthread_detach(hilo_procesar_cpu_memoria);

    while(1); // Memoria solo termina con Ctrl + C. Este while(1) hace que no termine por causas naturales.
    return 0;
}