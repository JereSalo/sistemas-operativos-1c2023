#ifndef PROCESAR_KERNEL_H
#define PROCESAR_KERNEL_H

#include "shared.h"

void esperar_clientes_kernel(int server_socket, t_log *logger);
void procesar_conexion_kernel(void* void_args);

typedef struct{
    t_log* log;
    int fd;
} t_procesar_conexion_kernel_args;


#endif