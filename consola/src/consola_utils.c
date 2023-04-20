#include "consola_utils.h"

/* ---------------------------------- PARSER ---------------------------------- */

void parsear_instrucciones(char* path_instrucciones, t_list* lista_instrucciones, t_log* logger) {

    // Vamos a leer el archivo y pasar todo a un solo string para trabajar con mayor comodidad
    char* archivo_string = archivo_a_string(path_instrucciones, logger);
    log_info(logger, "%s", archivo_string);

    // Spliteamos cada linea del string para obtener OPERACION (PARAM) ^n con n>=0
    char** instrucciones = string_split(archivo_string, '\n');
    log_info(logger, "%s", instrucciones[0]);
    log_info(logger, "%s", instrucciones[1]);
    
    // Spliteamos cada instruccion para separar la operacion de sus parametros y armamos la lista

    // ACA VAMOS A ARMAR LA LISTA DEFINITIVA
    
    

  /*   armar_lista_instrucciones(void* args)
    
    void _f_aux(void* instruccion_recibida){									// _f_aux recibe una instruccion de la lista de instrucciones (Mas abajo en el list_iterate) y envia a serializarla. Una vez que lo hace agrega al offset para desplazarse la cantidad de espacio que ocupo esa instruccion.
		char** separacion = string_split(instruccion, " ");
       //[ins, para, param] 
       list_iterate(separacion, add_List);
       instruccion* ins;
       ins->op_code_instruccion = separacion[0];
       ins->parametros = //asignar
       
       separacion[0]
	}

	list_iterate(lista_instrucciones, _f_aux);									// Cada instruccion en la lista de instrucciones va a realziar la funcion auxiliar
	return buffer;

    list_iterate(instrucciones, armar_lista_instrucciones(args));
    
     */
    
    
    free(instrucciones);
    free(archivo_string);
}


char* archivo_a_string(char* path_instrucciones, t_log* logger) {

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

void armar_lista_instrucciones(char** instrucciones, t_list* lista_instrucciones) {

    // recorrer lista y splitear
    //list_iterate(instrucciones, _f_aux);	// Cada instruccion en la lista de instrucciones va a realziar la funcion auxiliar
    //La funcion aux lo que podria hacer es armar la lista en donde cada elemento es una instruccion con la lista de param
}

long int calcular_tamanio_archivo(FILE * archivo){
	fseek(archivo, 0, SEEK_END);
	long int tamanio_archivo = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	return tamanio_archivo;
}