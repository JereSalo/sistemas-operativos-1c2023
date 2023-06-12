#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "kernel_utils.h"

// TODO

void planificador_largo_plazo();
void planificador_corto_plazo();


void mandar_a_ready(t_pcb* proceso);
void volver_a_running();
void mandar_a_running(t_pcb* proceso);

#endif

