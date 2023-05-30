#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include "shared.h"

typedef enum {
    NUMERO,
    INSTRUCCIONES,
    CONTEXTO_EJECUCION,
    PROCESO_DESALOJADO,
    STRING
} op_code;

void* serializar_instrucciones(size_t* size, t_list* instrucciones);
void* serializar_lista_instrucciones(size_t* size_instrucciones, t_list* lista_instrucciones);
void deserializar_instrucciones(void* stream, size_t size_instrucciones, t_list* instrucciones_recibidas, size_t* desplazamiento);

void* serializar_contexto(size_t* size, t_contexto_ejecucion* contexto);
void deserializar_contexto(void* stream, size_t stream_size, t_contexto_ejecucion* contexto, size_t* desplazamiento);

void* serializar_desalojo(size_t* size, int motivo_desalojo, t_list* lista_parametros);
void deserializar_desalojo(void* stream, size_t stream_size, int* motivo_desalojo, t_list* lista_parametros, size_t* desplazamiento);

void* serializar_numero(int numero);
void deserializar_numero(void* stream, int* numero);



#endif