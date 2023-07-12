#ifndef EJECUTAR_H_
#define EJECUTAR_H_

#include "cpu_utils.h"
#include "mmu.h"

// ------------------------------ EJECUCION DE PROCESOS ------------------------------ //

extern int desalojado;
extern int seg_fault;
extern t_list* lista_parametros;

void ejecutar_proceso(t_contexto_ejecucion* contexto);
void ejecutar_instruccion(char** instruccion_decodificada, t_contexto_ejecucion* contexto);

#endif