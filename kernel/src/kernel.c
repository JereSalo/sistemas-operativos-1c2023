#include "../include/kernel.h"

int main(int argc, char** argv){
    
    int modulo = KERNEL;
    t_log *logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    t_config *config = config_create("kernel.config");



    // CLIENTES -> CPU, Memoria y File System
    
    // Conexión con la CPU
    //int conexion_cpu = conectar_con(CPU, config, logger);

    // Conexión con FileSystem
    //int conexion_fs = conectar_con(FILESYSTEM, config, logger);

    // Conexión con Memoria
    //int conexion_mem = conectar_con(MEMORIA, config, logger);
   
    
    
    
    
    
    // SERVER -> conexión con la consola

    int server_fd = preparar_servidor(modulo, config, logger);


    // se queda escuchando -> mientras tenga clientes va a retornar 1

    while(server_escuchar(server_fd, logger, modulo));

    liberar_conexion(&server_fd);
    cerrar_programa(logger);

    return 0;
}


