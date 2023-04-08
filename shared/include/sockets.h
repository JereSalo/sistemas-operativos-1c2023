#ifndef SOCKETS_H_
#define SOCKETS_H_
#define _GNU_SOURCE

/*
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
*/

#include "shared.h"

int iniciar_servidor(char*, char*, t_log*, const char*);
int esperar_cliente(int, t_log*, const char*);
int crear_conexion(t_log* , const char* , char* , char*);
int recibir_operacion(int);
void liberar_conexion(int);


#endif


