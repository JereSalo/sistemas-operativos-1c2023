#include "memoria.h"

int main(int argc, char** argv){
    logger = log_create("memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);

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

    cliente_filesystem = esperar_cliente(server_fd, logger, "Memoria");
    cliente_cpu = esperar_cliente(server_fd, logger, "Memoria");
    cliente_kernel = esperar_cliente(server_fd, logger, "Memoria");
    

    // aca crea el hilo de "responder_orden()"

    liberar_conexion(&server_fd);
    cerrar_programa(logger,config);

    return 0;
}