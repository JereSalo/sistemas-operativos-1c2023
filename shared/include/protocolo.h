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

bool send_contexto_desalojado(int fd, t_contexto_ejecucion* contexto, char* motivo_desalojo);
bool recv_contexto_desalojado(int fd, t_contexto_ejecucion* contexto, char* motivo_desalojo);

bool recv_string(int fd, char* string);
bool send_string(int fd, char* string);

#endif