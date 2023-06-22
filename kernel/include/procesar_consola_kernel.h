#ifndef PROCESAR_CONSOLA_KERNEL_H_
#define PROCESAR_CONSOLA_KERNEL_H_

#include "kernel_utils.h"

// ------------------------------ PROCESAMIENTO DE CONSOLA EN KERNEL ------------------------------ //
void procesar_consola(void* void_cliente_consola);


// Funciones de otros modulos
t_pcb* inicializar_pcb(int cliente_consola); // esto esta declarado en inicializar.h -> RARITO


#endif