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

bool send_numero(int fd, int numero);
bool recv_numero(int fd, int* numero);

void* serializar_numero(int numero);
void deserializar_numero(void* stream, int* numero);

bool send_instrucciones(int fd, char** instrucciones);
bool recv_instrucciones(int fd, char** instrucciones);



//void procesar_conexion(t_log*, int, char*);
void procesar_conexion(void* void_args);


typedef enum {
    NUMERO,
    INSTRUCCIONES
} op_code;


// PUEDE SER QUE ESTO LO PONGAMOS EN OTRO .h (CAPAZ EN EL DE SOCKETS)
typedef struct{
    t_log* log;
    int fd;
    char* server_name;
} t_procesar_conexion_args;

#endif