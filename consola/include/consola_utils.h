#ifndef CONSOLA_UTILS_H
#define CONSOLA_UTILS_H

#include "shared.h"

// ------------------------------ VARIABLES GLOBALES ------------------------------ //

// LOGGER Y CONFIG
extern t_log* logger;
extern t_config* config;

// SOCKETS
extern int server_kernel;


// ------------------------------ PARSER ------------------------------ //
t_list* generar_lista_instrucciones(char* archivo_string);

#endif