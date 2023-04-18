#include "../include/kernel.h"

int main(int argc, char** argv){
    
    int modulo = KERNEL;
    t_log *logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    t_config *config = config_create("kernel.config");


    // SERVER -> conexión con la consola

    int server_fd = preparar_servidor(modulo, config, logger);

    // TENER EN CUENTA QUE PRIMERO TENEMOS QUE CONECTAR EL KERNEL AL CPU ANTES DE PRENDER EL SERVER 


    // Conexión con la CPU
    int conexion_cpu = conectar_con(CPU, config, logger);


    // se queda escuchando -> mientras tenga clientes va a retornar 1

    server_escuchar(server_fd, logger, "Kernel");

    // Guarda con lo que ponemos después de esto, el escuchar va después de haber realizado las conexiones, porque el server se queda escuchando infinitamente (hasta que algo falle).
    // Por esa razón escuchamos al final de todo.

    // CLIENTES -> CPU, Memoria y File System
    
    

    // Conexión con FileSystem
    //int conexion_fs = conectar_con(FILESYSTEM, config, logger);

    // Conexión con Memoria
    //int conexion_mem = conectar_con(MEMORIA, config, logger);
   

    // ENVIO DE MENSAJES

    int numero;
    printf("Ingrese un numero: ");
    scanf("%d", &numero);
    send_numero(conexion_cpu, numero);

    liberar_conexion(&server_fd);
    cerrar_programa(logger, config);

    return 0;
}


