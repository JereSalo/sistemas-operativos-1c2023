#ifndef PROCESAR_FS_KERNEL_H_
#define PROCESAR_FS_KERNEL_H_

#include "kernel_utils.h"

// ------------------------------ PROCESAMIENTO DE CPU EN KERNEL ------------------------------ //
void procesar_fs_kernel();

// Funciones usadas de otros modulos
void mandar_a_ready(t_pcb* proceso);
void volver_a_running();

#endif