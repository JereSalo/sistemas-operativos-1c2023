#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

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

#include "sharedUtils.h"
#include "serializacion.h"

typedef struct {
    int pid;                            // process id: identificador del proceso.
    //int size;
    int pc;                             // program counter: número de la próxima instrucción a ejecutar.
	//int cliente_socket;
    t_registros_cpu registros_cpu;
    t_list* instrucciones;              // lista de instrucciones a ejecutar. t_list*
	pcb_estado estado;
	t_list* tabla_segmentos;            // va a contener elementos de tipo t_segmento
    double estimacion_prox_rafaga;
    double tiempo_llegada_ready;
    t_list* tabla_archivos_abiertos;    // va a contener elementos de tipo FILE*
} t_pcb;

typedef struct {
    uint8_t registros_cpu_8[4];     // AX, BX, CX, DX
    uint16_t registros_cpu_16[4];   // EAX, EBX, ECX, EDX
    uint32_t registros_cpu_32[4];   // RAX, RBX, RCX, RDX
} t_registros_cpu;

typedef enum { // Los estados que puede tener un PCB
    NEW,
    READY,
    BLOCKED,
    RUNNING,
    EXIT,
} pcb_estado;

// TODO
typedef struct {
    int id;
    int direccion_base; // ???
	int size; // Tamaño de segmento
} t_segmento;