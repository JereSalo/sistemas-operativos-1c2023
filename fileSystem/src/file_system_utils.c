#include "file_system_utils.h"
#define FILE_PATH_MAX 4096

t_log* logger;
t_config* config;
t_superbloque info_superbloque;
t_filesystem_config config_filesystem;
int cliente_kernel;
int server_memoria;


t_config* archivo_superbloque;  //solo lo accedemos para obtener el tamanio y cantidad de bloques
int archivo_bitmap;             //con int porque lo mapeamos a memoria
int archivo_bloques;
size_t tamanio_archivo_bloques;
size_t tamanio_archivo_bitmap;
void* archivo_bloques_mapeado;
void* archivo_bitmap_mapeado;
t_bitarray* bitarray_bloques;

t_list* lista_fcbs;




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

    log_debug(logger, "Info del superbloque cargada - BLOCK_SIZE: %d   BLOCK_COUNT: %d \n", info_superbloque.BLOCK_SIZE, info_superbloque.BLOCK_COUNT); 
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
            tamanio = info_superbloque.BLOCK_COUNT / 8;
        }
        
        *tamanio_archivo = (size_t)tamanio;
        
        // Seteamos al archivo creado el tamanio que obtuvimos
        if (ftruncate(*archivo, tamanio) == -1) {	
            perror("Error seteando el tamanio de archivo");
            close(*archivo);
            return EXIT_FAILURE;
        }

        

    }

    return 0;
}


void mapear_archivo(char* tipo_archivo) {

    log_debug(logger, "Mapeando archivo a memoria \n");
    
    if(string_equals_ignore_case(tipo_archivo, "bloques")) {
        
        archivo_bloques_mapeado = mmap(NULL, tamanio_archivo_bloques, PROT_READ | PROT_WRITE, MAP_SHARED, archivo_bloques, 0);  //Con PROT_READ y PROT_WRITE estamos permitiendo que el archivo sea leido y escrito
        if (archivo_bloques_mapeado == MAP_FAILED) {
            perror("Error mapeando el archivo a memoria");
            close(archivo_bloques);
            exit(1);
        }

    }
    else if(string_equals_ignore_case(tipo_archivo, "bitmap")) {

        archivo_bitmap_mapeado = mmap(NULL, tamanio_archivo_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, archivo_bitmap, 0);  //Con PROT_READ y PROT_WRITE estamos permitiendo que el archivo sea leido y escrito
        if (archivo_bitmap_mapeado == MAP_FAILED) {
            perror("Error mapeando el archivo a memoria");
            close(archivo_bitmap);
            exit(1);
        }

        //Creamos el bitarray
                
        bitarray_bloques = bitarray_create_with_mode(archivo_bitmap_mapeado, info_superbloque.BLOCK_COUNT, LSB_FIRST);
    
    }

}

void desmapear_archivo(void* archivo_mapeado, size_t tamanio_archivo) {
    
    log_debug(logger, "Desmapeando archivo de memoria \n");
    
    if (munmap(archivo_mapeado, tamanio_archivo) == -1) {
        perror("Error unmapping the file");
        exit(1);
    }    
}

void sincronizar_archivo(void* archivo_mapeado, size_t tamanio_archivo) {

    log_debug(logger, "Sincronizando archivo mapeado a memoria con disco \n");
    
    if (msync(archivo_mapeado, tamanio_archivo, MS_SYNC) == -1) {
        perror("Error synchronizing data with file");
        exit(1);
    }   
}

void mostrar_bitarray() {

    int valor;
    
    log_debug(logger, "Mostrando bitarray en memoria: \n");
    
    for(int i = 0; i < 24; i++) {
        valor = bitarray_test_bit(bitarray_bloques, i);
        printf("Valor del bitarray en el bloque %d - %d \n", i, valor);
    }
}

// Este sirve solo para ver el contenido del archivo de bloques
void mostrar_contenido_archivo(char* path_archivo) {

    log_info(logger, "Mostrando data del archivo mapeado en el archivo original: \n");

    FILE* archivo = fopen(path_archivo, "r");

    if (archivo == NULL) {
        perror("Error opening file");
        exit(1);
    }

    int c;
    while ((c = fgetc(archivo)) != EOF) {
        putchar(c); 
    }

    fclose(archivo);

}

void mostrar_punteros_archivo_bloques() {

    log_debug(logger, "Mostrando contenido archivo de bloques: \n");
    uint32_t* data_as_chars = (uint32_t*)archivo_bloques_mapeado;
    for (int i = 0; i < 120; i++) {
        int nro_bloque = floor((float) i / (info_superbloque.BLOCK_SIZE / sizeof(uint32_t)));
        //if(data_as_chars[i] != 0)
        printf("VALOR ESCRITO EN BLOQUE NRO %d - %u \n", nro_bloque, data_as_chars[i]);
    }
}

void mostrar_archivo_mapeado_bloques() {

    log_debug(logger, "Mostrando contenido archivo de bloques mapeado: \n");
    char* data_as_chars = (char*)archivo_bloques_mapeado;
    for (int i = 0; i < 120; i++) {
        int nro_bloque = floor((float) i / (info_superbloque.BLOCK_SIZE / sizeof(uint32_t)));
        //if(data_as_chars[i] != 0)
        printf("VALOR ESCRITO EN BLOQUE NRO %d - %c \n", nro_bloque, data_as_chars[i]);
    }
}





void crear_estructuras_administrativas() {

    lista_fcbs = list_create();
    
    
    // hay que recorrer el directorio de FCB y si hay archivos dentro, vamos creando la lista de FCBs
    // si no hay FCBs, simplemente creamos una lista vacia -> se ira llenando a medida que se ejecuten instrucciones

    DIR* dir;
    struct dirent* entry;

    dir = opendir(config_filesystem.PATH_FCB);
    if (dir == NULL) {
        perror("Error al abrir el directorio.");
        exit(1);
    }


    while((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".dat") != NULL) {
            // Process only regular files with .dat extension

            // Create the full file path
            char file_path[FILE_PATH_MAX];

            // Concatena en una variable e imprime dicha variable
            snprintf(file_path, FILE_PATH_MAX, "%s/%s", config_filesystem.PATH_FCB, entry->d_name);

            log_debug(logger, "PATH: %s", file_path);

            
            // Accedemos como al config y creamos una entrada del tipo t_fcb, y la guardamos en la lista


            // Read the .dat file and process key-value pairs
            t_fcb* fcb = malloc(sizeof(t_fcb));
            
            t_config* archivo_fcb = config_create(file_path);

            fcb->nombre = strdup(config_get_string_value(archivo_fcb, "NOMBRE_ARCHIVO"));
            fcb->tamanio = config_get_int_value(archivo_fcb, "TAMANIO_ARCHIVO");
            fcb->puntero_directo = config_get_int_value(archivo_fcb, "PUNTERO_DIRECTO");
            fcb->puntero_indirecto = config_get_int_value(archivo_fcb, "PUNTERO_INDIRECTO");

            // Hacer funcion de mostrar lista afuera de esta funcion para chequear que este todo OK
            
            log_debug(logger, "Se creo un FCB - Nombre: %s - Tamanio: %d - PD: %d - PI: %d", fcb->nombre, fcb->tamanio, fcb->puntero_directo, fcb->puntero_indirecto);

            list_add(lista_fcbs, fcb);
            config_destroy(archivo_fcb);
        }

    }

    free(dir);
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



// ------------------------------ MANEJO DE ARCHIVOS ------------------------------ //

t_fcb* buscar_archivo_en_lista_fcbs(char* archivo_solicitado) {
    t_list_iterator* lista_it = list_iterator_create(lista_fcbs);

    while (list_iterator_has_next(lista_it)) {
        t_fcb* archivo = (t_fcb*)list_iterator_next(lista_it);
        
        if (string_equals_ignore_case(archivo->nombre, archivo_solicitado)) {
            list_iterator_destroy(lista_it);
            return archivo;
        }
    }
    
    list_iterator_destroy(lista_it);
    return NULL;
}

void crear_entrada_directorio(char* nombre_archivo) {

        char path_archivo[FILE_PATH_MAX];
        char nombre_concatenado[FILE_PATH_MAX];

        snprintf(path_archivo, FILE_PATH_MAX, "%s/%s.dat", config_filesystem.PATH_FCB, nombre_archivo);
        snprintf(nombre_concatenado, FILE_PATH_MAX, "NOMBRE_ARCHIVO=%s", nombre_archivo);

        write_to_dat_file(path_archivo, nombre_concatenado);
        write_to_dat_file(path_archivo, "TAMANIO_ARCHIVO=0");
        write_to_dat_file(path_archivo, "PUNTERO_DIRECTO=-1");
        write_to_dat_file(path_archivo, "PUNTERO_INDIRECTO=-1");

}


void write_to_dat_file(const char* filename, const char* data) {
    FILE* file = fopen(filename, "a"); // Open the file in append mode

    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Write the data to the file as text
    fprintf(file, "%s\n", data);

    fclose(file); // Close the file when done
}


