#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_



#include "shared_utils.h"
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
} t_procesar_conexion_args;

#endif