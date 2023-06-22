#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "kernel_utils.h"

// ------------------------------ PLANIFICADOR LARGO PLAZO ------------------------------ //
void planificador_largo_plazo();
void mandar_a_ready(t_pcb* proceso);

// ------------------------------ PLANIFICADOR CORTO PLAZO ------------------------------ //
void planificador_corto_plazo();
void volver_a_running();
void mandar_a_running(t_pcb* proceso);

#endif

