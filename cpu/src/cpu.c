#include "cpu.h"


int main(int argc, char** argv){
    int modulo = CPU;
    logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);
    t_config *config = config_create("cpu.config");

    cargar_config_cpu(config);
    
    inicializar_diccionarios();
    

    // CLIENTE -> Memoria
    // int conexion = conectar_con(MEMORIA, config, logger);

    // SERVER -> Kernel
    int server_fd = preparar_servidor(modulo, config, logger);

    int cliente_fd = esperar_cliente(server_fd, logger, "CPU");

    
    //Manejo de conexion con el Kernel -> recibe los contextos de ejecucion
    
    //pthread_t hilo;
    //pthread_create(&hilo, NULL, (void *)procesar_conexion_cpu, (void *) (intptr_t) cliente_fd);
    //pthread_detach(hilo);
    
    
    
    procesar_conexion_cpu(cliente_fd);

    liberar_conexion(&server_fd);
    cerrar_programa(logger,config);

    return 0;
}