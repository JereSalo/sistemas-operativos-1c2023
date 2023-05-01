#include "../include/kernel.h"

int main(int argc, char** argv){
    int modulo = KERNEL;
    t_log *logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    t_config *config = config_create("kernel.config");


    // Kernel es server del módulo Consola.
    int server_fd = preparar_servidor(modulo, config, logger);


    // CLIENTES -> CPU, Memoria y File System

    // Conexión con la CPU
    //int conexion_cpu = conectar_con(CPU, config, logger);

    // Conexión con FileSystem
    //int conexion_fs = conectar_con(FILESYSTEM, config, logger);

    // Conexión con Memoria
    //int conexion_mem = conectar_con(MEMORIA, config, logger);

    esperar_clientes_kernel(server_fd, logger);   

    cerrar_programa(logger, config);

    return 0;
}