#ifndef FS_UTILS_H
#define FS_UTILS_H

#include "shared.h"

typedef struct {
    char* IP_MEMORIA;
    int PUERTO_MEMORIA;
    int PUERTO_ESCUCHA;
    char* PATH_SUPERBLOQUE;
    char* PATH_BITMAP;
    char* PATH_BLOQUES;
    char* PATH_FCB;
    int RETARDO_ACCESO_BLOQUE;
} t_filesystem_config;

extern t_log* logger;
extern t_config *config;
extern t_filesystem_config config_filesystem;
extern int cliente_kernel;
extern int server_memoria;

void cargar_config_filesystem(t_config* config);

#endif