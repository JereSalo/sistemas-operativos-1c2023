#ifndef CONSOLA_UTILS_H
#define CONSOLA_UTILS_H

#include "shared.h"

char* leer_archivo(char* path_instrucciones, t_log* logger);
long int calcular_tamanio_archivo(FILE*);
t_list* generar_lista_instrucciones(char* archivo_string);


#endif