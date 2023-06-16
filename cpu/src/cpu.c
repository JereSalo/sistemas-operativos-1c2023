#include "cpu.h"


int main(int argc, char** argv){
    logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);
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
    int conexion = conectar_con(MEMORIA, config, logger);

    // SERVER -> Kernel
    int server_fd = preparar_servidor("CPU", config, logger);

    cliente_kernel = esperar_cliente(server_fd, logger, "CPU");   
    
    procesar_kernel();
    

    liberar_conexion(&server_fd);
    cerrar_programa(logger,config);

    return 0;
}