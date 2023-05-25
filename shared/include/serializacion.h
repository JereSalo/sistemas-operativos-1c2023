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

void* serializar_numero(int numero);
void deserializar_numero(void* stream, int* numero);

void* serializar_contexto_desalojado(size_t* size, t_contexto_ejecucion* contexto, char* motivo_desalojo);
void deserializar_contexto_desalojado(void* stream, size_t stream_size, t_contexto_ejecucion* contexto, size_t* desplazamiento, char* motivo_desalojo);

void* serializar_string(size_t* size, char* string);
void* deserializar_string(void* stream, char* string);

#endif