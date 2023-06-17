#ifndef MMU_H_
#define MMU_H_

#include "cpu_utils.h"

int traducir_direccion(int direccion_logica, t_contexto_ejecucion* proceso);
t_segmento* buscar_segmento(t_contexto_ejecucion* proceso, int num_segmento);

#endif