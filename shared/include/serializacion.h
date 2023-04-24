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

void tamanio_lista(size_t* size, t_list* lista);
void mostrar_lista(t_list* lista);
void* serializar_lista_instrucciones(size_t* size, t_list* lista_instrucciones);
void* sumarSizeConLongitudString(void* a, void* b);
size_t tamanio_lista_2(t_list* lista);
void deserializar_instrucciones(void* stream, t_list* instrucciones_recibidas);


#endif