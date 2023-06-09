#ifndef INICIALIZAR_H_
#define INICIALIZAR_H_

#include "kernel_utils.h"

void cargar_config_kernel(t_config* config);

void inicializar_semaforos();
void inicializar_colas();

t_pcb* inicializar_pcb(int cliente_consola);
t_pcb* crear_pcb(int pid, t_list* lista_instrucciones, int cliente_consola);


void inicializar_registros(t_registros_cpu* registros);

void inicializar_recursos();

#endif