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






void cargar_config_cpu(t_config* config);
void procesar_conexion_cpu(int);
void ejecutar_proceso(t_contexto_ejecucion* contexto, int cliente_socket);
void ejecutar_instruccion(char** instruccion_decodificada, t_contexto_ejecucion* contexto);
// void parametros_instruccion(char** instruccion_decodificada, char *parametros);
// void guardar_contexto(t_contexto_ejecucion* contexto);


#endif