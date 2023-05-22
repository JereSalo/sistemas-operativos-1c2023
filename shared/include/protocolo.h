#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

#include "shared.h"

void procesar_conexion(void* void_args);

bool send_numero(int fd, int numero);
bool recv_numero(int fd, int* numero);

bool send_instrucciones(int fd, t_list* instrucciones);
bool recv_instrucciones(int fd, t_list* instrucciones_recibidas);

bool send_contexto(int fd, t_contexto_ejecucion* contexto);
bool recv_contexto(int fd, t_contexto_ejecucion* contexto);



#endif