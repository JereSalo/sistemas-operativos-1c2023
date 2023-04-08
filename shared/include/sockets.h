#ifndef SOCKETS_H_
#define SOCKETS_H_
#define _GNU_SOURCE

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

int iniciar_servidor(char* ip, char* puerto, t_log* logger, const char* name);
int esperar_cliente(int socket_servidor, t_log* logger, const char* name);
int crear_conexion(t_log* logger, const char* server_name, char* ip, char* puerto);
void liberar_conexion(int socket_cliente);

#endif


