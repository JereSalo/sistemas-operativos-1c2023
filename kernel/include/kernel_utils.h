#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include "shared.h"

extern t_log* logger;

// Estados de procesos
//extern t_queue* procesos_en_new;
//extern t_list* procesos_en_ready;

// Semáforos
/* pthread_mutex_t mutex_new;
pthread_mutex_t mutex_ready;
sem_t maximo_grado_de_multiprogramacion;
sem_t cant_procesos_new; */

/* void inicializar_semaforos(t_config* config);
void inicializar_colas(); */

void esperar_clientes_kernel(int server_socket);
void procesar_conexion_kernel(void* cliente_socket);
t_pcb* crear_pcb(int pid, t_list* lista_instrucciones);
t_pcb* inicializar_pcb(int cliente_socket);


#endif