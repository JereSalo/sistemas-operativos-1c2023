#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "kernel_utils.h"

// TODO

void planificador_largo_plazo();
void planificador_corto_plazo(int);


void mandar_a_ready(t_pcb* proceso);
void volver_a_running();

#endif

