#ifndef PROCESAR_CPU_KERNEL_H_
#define PROCESAR_CPU_KERNEL_H_

#include "kernel_utils.h"

// Funciones de este modulo

void procesar_cpu();
void manejar_proceso_desalojado(op_instruccion motivo_desalojo, t_list* lista_parametros);
void mostrar_tabla_segmentos(t_list* tabla_segmentos);

// Funciones usadas de otros modulos

void mandar_a_ready(t_pcb* proceso);
void volver_a_running();


#endif