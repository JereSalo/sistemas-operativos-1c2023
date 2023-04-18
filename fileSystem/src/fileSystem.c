#include "../include/fileSystem.h"

int main(int argc, char** argv){
    int modulo = FILESYSTEM;
    
    t_log *logger = log_create("filesystem.log", "FILESYSTEM", true, LOG_LEVEL_INFO);
    t_config *config = config_create("filesystem.config");
    
    // CLIENTE -> Memoria
    // int conexion = conectar_con(MEMORIA, config, logger);


    // SERVER
    int server_fd = preparar_servidor(modulo, config, logger);

    server_escuchar(server_fd, logger, "fileSystem");

    liberar_conexion(&server_fd);
    cerrar_programa(logger,config);

    return 0;
}