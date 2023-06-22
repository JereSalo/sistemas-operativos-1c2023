#ifndef INICIALIZAR_H_
#define INICIALIZAR_H_

#include "kernel_utils.h"

// ------------------------------ CONFIG KERNEL ------------------------------ //
void cargar_config_kernel(t_config* config);
t_algoritmo_planificacion obtener_algoritmo_planificacion(char* string_algoritmo);

// ------------------------------ INICIALIZACION KERNEL ------------------------------ //
void inicializar_semaforos();
void inicializar_colas();
void inicializar_registros(t_registros_cpu* registros);
void inicializar_recursos();

// ------------------------------ PCB ------------------------------ //
t_pcb* inicializar_pcb(int cliente_consola);
t_pcb* crear_pcb(int pid, t_list* lista_instrucciones, int cliente_consola, t_list* tabla_segmentos);



#endif