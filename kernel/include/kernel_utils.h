#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include "shared.h"

extern t_log* logger;

// declaramos :) las colas de los estados
//t_queue* cola_new;
//t_queue* cola_ready;
//t_queue* cola_blocked;


t_pcb* inicializar_pcb(int cliente_socket);
void esperar_clientes_kernel(int server_socket);
void procesar_conexion_kernel(void* cliente_socket);


#endif