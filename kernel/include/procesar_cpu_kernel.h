#ifndef PROCESAR_CPU_KERNEL_H_
#define PROCESAR_CPU_KERNEL_H_

#include "kernel_utils.h"

// ------------------------------ PROCESAMIENTO DE CPU EN KERNEL ------------------------------ //
void procesar_cpu_kernel();
void manejar_proceso_desalojado(op_instruccion motivo_desalojo, t_list* lista_parametros);

// Funciones usadas de otros modulos
void mandar_a_ready(t_pcb* proceso);
void volver_a_running();


#endif