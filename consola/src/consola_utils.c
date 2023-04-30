#include "consola_utils.h"

/* ---------------------------------- PARSER ---------------------------------- */


t_list* generar_lista_instrucciones(char* path_instrucciones){
    FILE* archivo_instrucciones = fopen(path_instrucciones, "r");
    
    if(archivo_instrucciones == NULL) {
        printf("No se pudo abrir archivo de instrucciones ;)\n"); // Deberia ser un log, me gustaria crear un log aparte !
    }
    
    t_list* instrucciones = list_create();

    char linea[50];
    char* token;

    while(fgets(linea,sizeof linea,archivo_instrucciones)!= NULL){
        token = strtok(linea, "\n"); // Para sacar el \n de la linea
        list_add(instrucciones, strdup(token)); // El strdup en sí leakea memoria, no es casi nada de memoria, no es importante pero bueno.
    }


    mostrar_lista(instrucciones); // Esto es para ver si ta todo bien :)

    fclose(archivo_instrucciones);

    return instrucciones;
}