#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include "shared.h"


typedef struct {
    
    char* IP_KERNEL;
	char* IP_MEMORIA;
    int PUERTO_MEMORIA;
    char* IP_FILESYSTEM;
    int PUERTO_FILESYSTEM;
    char* IP_CPU;
    int PUERTO_CPU;
    int PUERTO_ESCUCHA;
    char* ALGORITMO_PLANIFICACION;
    int ESTIMACION_INICIAL;
    int HRRN_ALFA;
    int GRADO_MAX_MULTIPROGRAMACION;
    //t_list* RECURSOS;
    //t_list* INSTANCIAS_RECURSOS;

} t_kernel_config;


extern t_kernel_config* config_kernel;
extern t_log* logger;
extern int pid_counter;

// Estados de procesos
extern t_queue* procesos_en_new;
extern t_list* procesos_en_ready;


// Semáforos
extern pthread_mutex_t mutex_new;
extern pthread_mutex_t mutex_ready;
extern sem_t maximo_grado_de_multiprogramacion;
extern sem_t cant_procesos_new;
extern sem_t cant_procesos_ready;


void cargar_config_kernel(t_config* config);

void inicializar_colas();
void inicializar_semaforos();

void esperar_clientes_kernel(int server_socket);
void procesar_conexion_kernel(void* cliente_socket);
t_pcb* crear_pcb(int pid, t_list* lista_instrucciones);
t_pcb* inicializar_pcb(int cliente_socket);


#endif