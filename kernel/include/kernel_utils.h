#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include "shared.h"

extern t_log* logger;

void esperar_clientes_kernel(int server_socket);
void procesar_conexion_kernel(void* cliente_socket);
t_pcb* crear_pcb(int pid, t_list* lista_instrucciones);
t_pcb* inicializar_pcb(int cliente_socket);

#endif