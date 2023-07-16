#include "file_system_utils.h"

t_log* logger;
t_config* config;
t_superbloque info_superbloque;
t_filesystem_config config_filesystem;
int cliente_kernel;
int server_memoria;
t_config* archivo_superbloque;  //solo lo accedemos para obtener el tamanio y cantidad de bloques
int archivo_bitmap;             //con int porque lo mapeamos a memoria
int archivo_bloques;



//FILE* archivo_superbloque;
//FILE* archivo_bitmap;
//FILE* archivo_bloques;

void cargar_config_filesystem(t_config* config){
    config_filesystem.IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    config_filesystem.PUERTO_MEMORIA = config_get_int_value(config, "PUERTO_MEMORIA");
    config_filesystem.PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
    config_filesystem.PATH_SUPERBLOQUE = config_get_string_value(config, "PATH_SUPERBLOQUE");
    config_filesystem.PATH_BITMAP = config_get_string_value(config, "PATH_BITMAP");
    config_filesystem.PATH_BLOQUES = config_get_string_value(config, "PATH_BLOQUES");
    config_filesystem.PATH_FCB = config_get_string_value(config, "PATH_FCB");
    config_filesystem.RETARDO_ACCESO_BLOQUE = config_get_int_value(config, "RETARDO_ACCESO_BLOQUE");

    log_info(logger, "Config cargada en filesystem");
}

void cargar_info_superbloque(t_config* archivo_superbloque) {
    info_superbloque.BLOCK_COUNT = config_get_int_value(archivo_superbloque, "BLOCK_COUNT");
    info_superbloque.BLOCK_SIZE = config_get_int_value(archivo_superbloque, "BLOCK_SIZE"); 
}


int levantar_archivo(char* path, int* archivo, size_t* tamanio_archivo, char* tipo_archivo) {

    const char* path_archivo = path;

    if(access(path_archivo, F_OK) == 0) {

        // El archivo existe y lo abrimos en modo lectura-escritura
        *archivo = open(path_archivo, O_RDWR);
        if(*archivo == -1) {
            perror("Error abriendo el archivo de bloques");
            return EXIT_FAILURE;
        }

        // Obtenemos el tamanio del archivo existente
        struct stat file_stat;
        if(fstat(*archivo, &file_stat) == -1) {
            perror("Error obteniendo el tamanio de archivo");
            close(*archivo);
            return EXIT_FAILURE;
        }
        // Guardamos el tamanio para despues hacer mmap afuera
        *tamanio_archivo = file_stat.st_size;

        // Ahora viene todo el tema del mmap -> lo vamos a hacer afuera de esta funcion
    }
    else {

        // El archivo no existe y debemos crearlo
        *archivo = open(path_archivo, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if(*archivo == -1) {
            perror("Error creando/abriendo el archivo de bloques");
            return EXIT_FAILURE;
        }

        // Obtenemos el tamanio que queremos que tenga el archivo
        off_t tamanio;

        if(string_equals_ignore_case(tipo_archivo, "bloques")) {
            tamanio = info_superbloque.BLOCK_COUNT * info_superbloque.BLOCK_SIZE;
            
        }
        else if(string_equals_ignore_case(tipo_archivo, "bitmap")) {
            //poner aca la logica de calcular el tamanio cuando es un bitmap
            //tamanio = ...
        }
        
        *tamanio_archivo = (size_t)tamanio;
        
        // Seteamos al archivo creado el tamanio que obtuvimos
        if (ftruncate(*archivo, tamanio) == -1) {	
            perror("Error seteando el tamanio de archivo");
            close(*archivo);
            return EXIT_FAILURE;
        }

        // Ahora viene todo el tema del mmap

    }

    return 0;
}




void crear_estructuras_administrativas() {


}








FILE* abrir_archivo_bitmap(){
    FILE* archivo_bitmap = fopen(config_filesystem.PATH_BITMAP, "w");
    if(archivo_bitmap == NULL){
        log_error(logger, "NO ");
        return NULL;
    }
    return archivo_bitmap;
}

FILE* abrir_archivo_bloques(){
    FILE* archivo_bloques = fopen(config_filesystem.PATH_BLOQUES, "w");
    if(archivo_bloques == NULL){
        log_error(logger, "Error al abrir archivo superbloque (No existe directorio?)");
        return NULL;
    }
    return archivo_bloques;
}
