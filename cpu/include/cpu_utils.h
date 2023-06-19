#ifndef CPU_UTILS_H
#define CPU_UTILS_H

#include "shared.h"

typedef struct {
    int RETARDO_INSTRUCCION;
    char* IP_MEMORIA;
    int PUERTO_MEMORIA;
    int PUERTO_ESCUCHA;
    int TAM_MAX_SEGMENTO;
} t_cpu_config;

extern t_cpu_config config_cpu;
extern t_log* logger;
extern int cliente_kernel;
extern int server_memoria;

void cargar_config_cpu(t_config* config);


#endif