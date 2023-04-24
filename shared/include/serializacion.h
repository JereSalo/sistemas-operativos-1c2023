#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

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

typedef enum {
    NUMERO,
    INSTRUCCIONES
} op_code;

void mostrar_lista(t_list* lista);
void* sumarSizeConLongitudString(void* a, void* b);
size_t tamanio_lista(t_list* lista);

void* serializar_instrucciones(size_t* size, t_list* instrucciones);
void* serializar_lista_instrucciones(size_t* size, t_list* lista_instrucciones);
void deserializar_instrucciones(void* stream, size_t stream_size, t_list* instrucciones_recibidas);


void* serializar_numero(int numero);
void deserializar_numero(void* stream, int* numero);



#endif