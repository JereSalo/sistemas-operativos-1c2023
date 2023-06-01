#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "kernel_utils.h"

// TODO

void planificador_largo_plazo();
void planificador_corto_plazo(int);
void cargar_contexto_de_ejecucion(t_pcb*, t_contexto_ejecucion*);

#endif

