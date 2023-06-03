#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

#include "shared_utils.h"
#include "serializacion.h"

void send_opcode(int fd, int opcode);
int recv_opcode(int fd);


bool send_numero(int fd, int numero);
bool recv_numero(int fd, int* numero);

bool send_instrucciones(int fd, t_list* instrucciones);
bool recv_instrucciones(int fd, t_list* instrucciones_recibidas);

bool send_contexto(int fd, t_contexto_ejecucion* contexto);
bool recv_contexto(int fd, t_contexto_ejecucion* contexto);

bool send_desalojo(int fd, int motivo_desalojo, t_list* lista_parametros);
bool recv_desalojo(int fd, int* motivo_desalojo, t_list* lista_parametros);

bool send_string(int fd, char* string);
bool recv_string(int fd, char* string);


// FUNCIONES AUXILIARES (Se usan dentro de otros recv)
bool send_paquete(int fd, void* paquete, size_t size_paquete);
void* recv_paquete(int fd, size_t size_paquete);

void* recv_payload_con_size(int fd, size_t* size_payload);

// MACROS
#define SEND_INT(socket, value) send(socket, &(int){value}, sizeof(int), 0)
#define RECV_INT(socket, value) recv(socket, &(value), sizeof(int), MSG_WAITALL)

#endif