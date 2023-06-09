#ifndef PROCESAR_CPU_H_
#define PROCESAR_CPU_H_

#include "kernel_utils.h"

// Funciones de este modulo

void procesar_cpu(void* void_server_cpu);
void manejar_proceso_desalojado(op_instruccion motivo_desalojo, t_list* lista_parametros);


// Funciones usadas de otros modulos

void volver_a_encolar_en_ready(t_pcb* proceso);
void volver_a_running();


#endif