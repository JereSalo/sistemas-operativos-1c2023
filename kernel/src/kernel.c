#include "kernel.h"

uint grado_de_multiprogramacion;

int main(int argc, char** argv){
    int modulo = KERNEL;
    logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    t_config *config = config_create("kernel.config");

    grado_de_multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");

    // Kernel es server del módulo Consola.
    int server_fd = preparar_servidor(modulo, config, logger);


    // CLIENTES -> CPU, Memoria y File System

    // Conexión con la CPU
    //int conexion_cpu = conectar_con(CPU, config, logger);

    // Conexión con FileSystem
    //int conexion_fs = conectar_con(FILESYSTEM, config, logger);

    // Conexión con Memoria
    //int conexion_mem = conectar_con(MEMORIA, config, logger);


    // inicializar semaforos y colas
    inicializar_semaforos(grado_de_multiprogramacion);
    inicializar_colas();

    // Creamos el hilo donde se ejecuta el planificador de largo plazo
    pthread_t hilo_planificador_largo;
	pthread_create(&hilo_planificador_largo, NULL, (void*)planificador_largo_plazo, NULL);
	pthread_detach(hilo_planificador_largo);
   


    // Hilo main espera clientes, por cada cliente que se conecta crea un hilo extra para procesar la conexión del mismo
    while (1)
    {
        int consola_fd = esperar_cliente(server_fd, logger, "Kernel");
        pthread_t hilo;
        pthread_create(&hilo, NULL, (void *)procesar_conexion_kernel, (void *) (intptr_t) consola_fd);
        pthread_detach(hilo);
    }

    cerrar_programa(logger, config);

    return 0;
}