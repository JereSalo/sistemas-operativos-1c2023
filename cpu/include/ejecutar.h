#ifndef EJECUTAR_H_
#define EJECUTAR_H_

#include "cpu_utils.h"
#include "mmu.h"

extern int desalojado;
extern t_list* lista_parametros;

void ejecutar_proceso(t_contexto_ejecucion* contexto);
void ejecutar_instruccion(char** instruccion_decodificada, t_contexto_ejecucion* contexto);

#endif