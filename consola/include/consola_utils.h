#ifndef CONSOLA_UTILS_H
#define CONSOLA_UTILS_H

#include "shared.h"

void parsear_instrucciones(char* path_instrucciones, t_list* lista_instrucciones, t_log* logger);
char* archivo_a_string(char* path_instrucciones, t_log* logger);
void armar_lista_instrucciones(char** instrucciones, t_list* lista_instrucciones);
long int calcular_tamanio_archivo(FILE*);
typedef struct {
    int op_code_instruccion;
    t_list* parametros;
} instruccion;



#endif