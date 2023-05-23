#ifndef SHARED_UTILS_H_
#define SHARED_UTILS_H_

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
#include <pthread.h>
#include "commons/collections/queue.h"
#include <semaphore.h>
#include <stdint.h>


typedef struct{
    uint8_t registros_cpu_8[4];     // AX, BX, CX, DX
    uint16_t registros_cpu_16[4];   // EAX, EBX, ECX, EDX
    uint32_t registros_cpu_32[4];   // RAX, RBX, RCX, RDX
} t_registros_cpu;

typedef struct {
    int id;
    int direccion_base; // ???
	int size; // Tamaño de segmento
} t_segmento;

typedef struct {
    int pid;  
    int pc; 
    t_registros_cpu registros_cpu;
    t_list* instrucciones; 
} t_contexto_ejecucion;



void mostrar_lista(t_list* lista);
void* sumarSizeConLongitudString(void* a, void* b);
size_t tamanio_lista(t_list* lista);

void copiar_variable_en_stream_y_desplazar(void* paquete, void* elemento, size_t tamanio_elemento, size_t* desplazamiento);
void copiar_en_stream_y_desplazar_lista_strings_con_tamanios(void* paquete, t_list* lista_instrucciones);
void copiar_stream_en_variable_y_desplazar(void* variable, void* stream, size_t tamanio_elemento, size_t* desplazamiento);

#endif