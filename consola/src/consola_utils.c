#include "consola_utils.h"


t_log* logger;
t_config* config;
int server_kernel;

/* ---------------------------------- PARSER ---------------------------------- */


t_list* generar_lista_instrucciones(char* path_instrucciones) {
    
    FILE* archivo_instrucciones = fopen(path_instrucciones, "r");
    
    if(archivo_instrucciones == NULL) {
        log_error(logger, "No se pudo abrir el archivo de instrucciones :(");
        exit(2);
    }
    
    t_list* instrucciones = list_create();

    char linea[50];
    char* token;

    while(fgets(linea,sizeof linea,archivo_instrucciones)!= NULL){
        token = strtok(linea, "\n"); // Para sacar el \n de la linea
        list_add(instrucciones, strdup(token));
    }

    fclose(archivo_instrucciones);

    return instrucciones;
}