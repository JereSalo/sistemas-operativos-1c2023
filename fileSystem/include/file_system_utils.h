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
extern size_t tamanio_archivo_bloques;
extern size_t tamanio_archivo_bitmap;
extern void* archivo_bloques_mapeado;
extern void* archivo_bitmap_mapeado;
extern t_bitarray* bitarray_bloques;

extern t_list* lista_fcbs;


//extern FILE* archivo_bitmap;
//extern FILE* archivo_bloques;

void cargar_config_filesystem(t_config* config);
void cargar_info_superbloque(t_config* archivo_superbloque);
int levantar_archivo(char* path, int* archivo, size_t* tamanio_archivo, char* tipo_archivo);
void mostrar_contenido_archivo(char* path_archivo);
void mapear_archivo(char* tipo_archivo);
void mostrar_bitarray();
void desmapear_archivo(void* archivo_mapeado, size_t tamanio_archivo);
void sincronizar_archivo(void* archivo_mapeado, size_t tamanio_archivo);
void crear_estructuras_administrativas();
void crear_entrada_directorio(char* nombre_archivo);

void write_to_dat_file(const char* filename, const char* data);

FILE* abrir_archivo_superbloque();
FILE* abrir_archivo_bitmap();
FILE* abrir_archivo_bloques();

t_fcb* buscar_archivo_en_lista_fcbs(char* archivo_solicitado);
void mostrar_archivo_mapeado_bloques();

void mostrar_punteros_archivo_bloques();


#endif