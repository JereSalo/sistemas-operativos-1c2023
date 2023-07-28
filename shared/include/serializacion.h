#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include "shared_utils.h"


typedef enum {
    NUMERO,
    INSTRUCCIONES,
    CONTEXTO_EJECUCION,
    PROCESO_DESALOJADO,
    STRING,
    SOLICITUD_TABLA_NEW,
    SOLICITUD_CREACION_SEGMENTO,
    SOLICITUD_ELIMINACION_SEGMENTO,
    SOLICITUD_COMPACTACION,
    SEGMENTOS,
    SOLICITUD_LECTURA,
    SOLICITUD_ESCRITURA,
    SOLICITUD_LIBERAR_MEMORIA,
    SOLICITUD_LECTURA_DISCO,
    SOLICITUD_ESCRITURA_DISCO,
    SOLICITUD_CREAR_ARCHIVO,
    SOLICITUD_TRUNCAR_ARCHIVO,
    SOLICITUD_ABRIR_ARCHIVO,
    SOLICITUD_CERRAR_ARCHIVO,
    RESPUESTA_FREAD,
    RESPUESTA_FWRITE,
    RESPUESTA_FOPEN,
    RESPUESTA_CREATE,
    RESPUESTA_FTRUNCATE
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

void deserializar_string(void* stream, size_t stream_size, char* string, size_t* desplazamiento);
void* serializar_string(size_t* size, char* string);

void* serializar_segmentos(size_t* size, t_list* segmentos);
void* serializar_tabla_segmentos(size_t* size_tabla_segmentos, t_list* tabla_segmentos);
void deserializar_segmentos(void* stream, size_t size_segmentos , t_list* tabla_segmentos, size_t* desplazamiento);


void* serializar_solicitud_creacion_segmento(size_t* size, int pid, int id_segmento, int tamanio_segmento);
void deserializar_solicitud_creacion_segmento(void* payload, int* pid, int* id_segmento, int* tamanio_segmento, size_t* desplazamiento);

void* serializar_peticion_lectura(size_t* size, int direccion_fisica, int longitud);
void* serializar_peticion_escritura(size_t* size, int direccion_fisica, int longitud, char* valor_leido);

void* serializar_solicitud_eliminacion_segmento(size_t* size, int id_segmento, int pid);
void deserializar_solicitud_eliminacion_segmento(void* payload, int* id_segmento, int* pid, size_t* desplazamiento);

void* serializar_segmentos_por_proceso(size_t* size, t_list* tabla_segmentos_por_proceso, int cant_segmentos);
void* serializar_tabla_segmentos_por_proceso(size_t* size_tabla_segmentos_por_proceso, t_list* tabla_segmentos_por_proceso, int cant_segmentos);
void deserializar_segmentos_por_proceso(void* stream, size_t size_segmentos , t_list* tabla_segmentos_por_proceso, size_t* desplazamiento, int cant_segmentos);

void* serializar_solicitud_tabla(int pid_counter);
void* serializar_base_segmento(int direccion_base);

#endif