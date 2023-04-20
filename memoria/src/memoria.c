#include "memoria.h"

int main(int argc, char** argv){
    int modulo = MEMORIA;

    t_log *logger = log_create("memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
    t_config *config = config_create("memoria.config");

    
    // SERVER -> CPU, Kernel, FileSystem
    int server_fd = preparar_servidor(modulo, config, logger);

    esperar_clientes(server_fd, logger, "Memoria");

    liberar_conexion(&server_fd);
    cerrar_programa(logger,config);

    return 0;
}