#include "file_system.h"

int main(int argc, char** argv){
    logger = log_create("filesystem.log", "FILESYSTEM", true, LOG_LEVEL_INFO);

    if (argc==1){
        config = config_create("filesystem.config");
    }
    else if (argc==2) {
        config = config_create(argv[1]); // Un ejemplo seria ../tests/BASE/filesystem.config
    }
    else{
        log_error(logger, "Cantidad de parametros incorrecta. Debería ser ./filesystem PATH_CONFIG \n"); 
        return EXIT_FAILURE;
    }

    cargar_config_filesystem(config);
    
    // CLIENTE -> Memoria
    server_memoria = conectar_con(MEMORIA, config, logger);

    // SERVER
    int server_fd = preparar_servidor("FILESYSTEM", config, logger);

    // Aca ya no mas esperar_clientes, esperamos individualmente.
    cliente_kernel = esperar_cliente(server_fd, logger, "fileSystem");



    liberar_conexion(&server_fd);
    cerrar_programa(logger,config);

    return 0;
}