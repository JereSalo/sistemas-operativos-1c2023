#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "kernel_utils.h"

// TODO

void planificador_largo_plazo();
void planificador_corto_plazo(int);
void cargar_contexto_de_ejecucion(t_pcb*, t_contexto_ejecucion*);
void calcular_tasa_de_respuesta(double tiempo_actual);
t_pcb* proceso_con_mayor_tasa_de_respuesta() ;


#endif

