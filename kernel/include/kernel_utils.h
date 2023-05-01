#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include "shared.h"

extern t_log* logger;

void esperar_clientes_kernel(int server_socket);
void procesar_conexion_kernel(void* cliente_socket);


#endif