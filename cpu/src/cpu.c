#include "cpu.h"

int main(int argc, char** argv){
    
    int modulo = CPU;
    t_log *logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);
    t_config *config = config_create("cpu.config");


    // CLIENTE -> Memoria
    int conexion = conectar_con(MEMORIA, config, logger);

    // SERVER -> Kernel
    int server_fd = preparar_servidor(modulo, config, logger);

    while(server_escuchar(server_fd, logger, "CPU"));

    liberar_conexion(&server_fd);
    cerrar_programa(logger);

    return 0;
}