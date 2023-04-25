#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>

#include "sharedUtils.h"
#include "serializacion.h"

void procesar_conexion(void* void_args);


bool send_numero(int fd, int numero);
bool recv_numero(int fd, int* numero);

bool send_instrucciones(int fd, t_list* instrucciones);
bool recv_instrucciones(int fd, t_list* instrucciones_recibidas);


// PUEDE SER QUE ESTO LO PONGAMOS EN OTRO .h (CAPAZ EN EL DE SOCKETS)
typedef struct{
    t_log* log;
    int fd;
    char* server_name;
} t_procesar_conexion_args;

#endif