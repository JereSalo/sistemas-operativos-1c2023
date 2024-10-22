#include "cpu.h"


int main(int argc, char** argv){
    logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_DEBUG);
    t_config *config;

    if (argc==1){
        config = config_create("cpu.config");
    }
    else if (argc==2) {
        config = config_create(argv[1]); // Un ejemplo seria ../tests/BASE/cpu.config
    }
    else{
        log_error(logger, "Cantidad de parametros incorrecta. Debería ser ./cpu PATH_CONFIG \n"); 
        return EXIT_FAILURE;
    }
    inicializar_diccionarios();

    cargar_config_cpu(config);
       
    // CLIENTE -> Memoria
    server_memoria = conectar_con(MEMORIA, config, logger);

    // Mandamos handshake
    //SEND_INT(server_memoria, 1);

    // Recibimos respuesta del handshake
    //int respuesta_handshake;
    //RECV_INT(server_memoria, respuesta_handshake);

    //if(respuesta_handshake == 2) {
    //   log_info(logger, "Handshake exitoso \n");
    //}

    // Mandamos a memoria el tamanio maximo de segmento permitido por CPU
    SEND_INT(server_memoria, config_cpu.TAM_MAX_SEGMENTO);
    log_debug(logger, "Envie a memoria el tamanio maximo de segmento: %d", config_cpu.TAM_MAX_SEGMENTO);

    // SERVER -> Kernel
    int server_fd = preparar_servidor("CPU", config, logger);

    cliente_kernel = esperar_cliente(server_fd, logger, "CPU");   
    
    procesar_kernel_cpu();
    

    liberar_conexion(&server_fd);
    cerrar_programa(logger,config);

    return 0;
}