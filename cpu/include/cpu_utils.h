#ifndef CPU_UTILS_H
#define CPU_UTILS_H

#include "shared.h"

extern t_log* logger;


typedef struct {
    int RETARDO_INSTRUCCION;
    char* IP_MEMORIA;
    int PUERTO_MEMORIA;
    int PUERTO_ESCUCHA;
    int TAM_MAX_SEGMENTO;
} t_cpu_config;

extern t_cpu_config config_cpu;
extern t_list* lista_parametros;
extern int cliente_kernel;


void cargar_config_cpu(t_config* config);
void procesar_kernel();
void ejecutar_proceso(t_contexto_ejecucion* contexto);
void ejecutar_instruccion(char** instruccion_decodificada, t_contexto_ejecucion* contexto);


#endif