#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "shared.h"
#include "kernel_utils.h"

extern t_log* logger;

// TODO
typedef struct {
    int pid;                            // process id: identificador del proceso.
    //int size;
    int pc;                             // program counter: número de la próxima instrucción a ejecutar.
	//int cliente_socket;
    t_registros_cpu registros_cpu;
    t_list* instrucciones;              // lista de instrucciones a ejecutar. t_list*
	t_list* tabla_segmentos;            // va a contener elementos de tipo t_segmento
    double estimacion_prox_rafaga;      // Para HRRN
    double tiempo_llegada_ready;        // Para HRRN
    t_list* tabla_archivos_abiertos;    // va a contener elementos de tipo FILE*
} t_pcb;



void planificador_largo_plazo(void*);
//void planificador_corto_plazo(void*, int);
void planificador_corto_plazo(int);
void cargar_contexto_de_ejecucion(t_pcb*, t_contexto_ejecucion*);


#endif

