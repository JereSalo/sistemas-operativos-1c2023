#include "../include/kernel.h"

int main(int argc, char** argv){
    int modulo = KERNEL;
    t_log *logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    t_config *config = config_create("kernel.config");


    // Kernel es server del módulo Consola.
    int server_fd = preparar_servidor(modulo, config, logger);

    // Conexión con la CPU
    //int conexion_cpu = conectar_con(CPU, config, logger);



    //FIJARSE DE CERRAR EL PROGRAMA CUANDO HACEMOS CONTROL + C -> memory leaks en esperar cliente

    //esperar_clientes(server_fd, logger, "Kernel");

    /*void sighandler(int s) {
        exit(0);
    }

    signal(SIGINT, sighandler);*/
    
    
    cerrar_programa(logger, config);
    // Guarda con lo que ponemos después de esto, el escuchar va después de haber realizado las conexiones, porque el server se queda escuchando infinitamente (hasta que algo falle).

    // CLIENTES -> CPU, Memoria y File System




    
    

    // Conexión con FileSystem
    //int conexion_fs = conectar_con(FILESYSTEM, config, logger);

    // Conexión con Memoria
    //int conexion_mem = conectar_con(MEMORIA, config, logger);
   

    // ENVIO DE MENSAJES

    

    

    return 0;
}


