#ifndef MMU_H_
#define MMU_H_

#include "cpu_utils.h"

// ------------------------------ TRADUCCION DE DIRECCIONES ------------------------------ //
t_segmento* buscar_segmento(t_contexto_ejecucion* proceso, int num_segmento);
int obtener_direccion(int direccion_logica, t_contexto_ejecucion* proceso, char* nombre_registro);

#endif