#ifndef CPU_UTILS_H
#define CPU_UTILS_H

#include "shared.h"

extern t_log* logger;
extern t_dictionary* diccionario_instrucciones;

typedef enum {
    SET,
    EXIT,
    YIELD
} op_instruccion;


void procesar_conexion_cpu(int);
void ejecutar_proceso(t_contexto_ejecucion* contexto);
void ejecutar_instruccion(char** instruccion_decodificada, t_contexto_ejecucion* contexto);

#endif