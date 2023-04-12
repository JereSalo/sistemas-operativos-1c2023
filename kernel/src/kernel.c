#include "../include/kernel.h"

int main(int argc, char** argv){
    
    int modulo = KERNEL;
    t_log *logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    t_config *config = config_create("kernel.config");

    // Clientes -> CPU, Memoria y File System
    
    // Conexión con la CPU
    int conexion = conectar_con(CPU, config, logger);

    
    
   
   
    
    // Server -> conexión con la consola
    int cliente_fd = iniciar_servidor_y_esperar_cliente(modulo, config, logger);

    //PROCESAR CONEXION -> RECIBIR MENSAJES -> HAY QUE HACER LOS SEND DEL LADO DEL CLIENTE ANTES
    procesar_conexion(logger, cliente_fd, "Kernel");


}


