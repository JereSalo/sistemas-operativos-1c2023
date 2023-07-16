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

typedef struct {
    int BLOCK_COUNT;
    int BLOCK_SIZE;
} t_superbloque;

extern t_log* logger;
extern t_config *config;
extern t_superbloque info_superbloque;
extern t_filesystem_config config_filesystem;
extern int cliente_kernel;
extern int server_memoria;
extern t_config* archivo_superbloque;
extern int archivo_bitmap;
extern int archivo_bloques;

//extern FILE* archivo_bitmap;
//extern FILE* archivo_bloques;

void cargar_config_filesystem(t_config* config);
void cargar_info_superbloque(t_config* archivo_superbloque);
int levantar_archivo(char* path, int* archivo, size_t* tamanio_archivo, char* tipo_archivo);


FILE* abrir_archivo_superbloque();
FILE* abrir_archivo_bitmap();
FILE* abrir_archivo_bloques();


#endif