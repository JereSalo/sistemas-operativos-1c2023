#include "consola_utils.h"

/* ---------------------------------- PARSER ---------------------------------- */

char* leer_archivo(char* path_instrucciones, t_log* logger) {

    //Abrimos el archivo
    FILE* archivo_instrucciones = fopen(path_instrucciones, "r");
    
    if(archivo_instrucciones == NULL) {
        log_error(logger, "NO SE PUDO ABRIR EL ARCHIVO DE INSTRUCCIONES");                   
    }
       
    long int tamanio_archivo = calcular_tamanio_archivo(archivo_instrucciones);

    // Reservamos memoria donde se va a guardar lo leido (le agregamos uno mas para guardar el EOF)                      
    char* archivo_string = malloc(tamanio_archivo + 1);                                 

    // guardamos en archivo_string lo leido
    fread(archivo_string, tamanio_archivo, 1, archivo_instrucciones);

    // Agregamos el EOF al string
    archivo_string[tamanio_archivo] = '\0';                                            

    fclose(archivo_instrucciones);

    return archivo_string;
}


long int calcular_tamanio_archivo(FILE * archivo){
	fseek(archivo, 0, SEEK_END);
	long int tamanio_archivo = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	return tamanio_archivo;
}

t_list* generar_lista_instrucciones(char* archivo_string) {
    char** instrucciones = string_split(archivo_string, "\n");
    t_list* lista_instrucciones = list_create();
    int i = 0;
    while(instrucciones[i] != NULL){
        list_add(lista_instrucciones,instrucciones[i]);
        i++;
    }

    //liberamos el char**

   string_array_destroy(instrucciones);



    return lista_instrucciones;
}