#include "consola_utils.h"

/* ---------------------------------- PARSER ---------------------------------- */


t_list* generar_lista_instrucciones(char* path_instrucciones){
    FILE* archivo_instrucciones = fopen(path_instrucciones, "r");
    
    if(archivo_instrucciones == NULL) {
        printf("No se pudo abrir archivo de instrucciones ;)\n"); // Deberia ser un log, me gustaria crear un log aparte !
    }
    
    t_list* instrucciones = list_create();

    char linea[50];

    while(fgets(linea,sizeof linea,archivo_instrucciones)!= NULL){
        // En esta altura linea (a menos que sea la última) sería la instrucción mas el enter del final, quiero sacar el enter.
        if(linea[strlen(linea)-1] == '\n'){linea[strlen(linea)-1] = '\0';} // Este if es nefasto pero bueno anda bien :)
        list_add(instrucciones, strdup(linea)); // El strdup en sí leakea memoria, no es casi nada de memoria, no es importante pero bueno.
    }


    mostrar_lista(instrucciones);
    //printf("%d",instrucciones->elements_count);
    fclose(archivo_instrucciones);

    return instrucciones;
}