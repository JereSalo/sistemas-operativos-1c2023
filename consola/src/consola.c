#include "consola.h"

// SOMOS EL CLIENTE

uint32_t convertir_instruccion(char* str){
	if(!strcmp(str, "NO_OP"))
		return NO_OP;
	if(!strcmp(str, "I/O"))
		return I_O;
	if(!strcmp(str, "READ"))
		return READ;
	if(!strcmp(str, "WRITE"))
		return WRITE;
	if(!strcmp(str, "COPY"))
		return COPY;
	if(!strcmp(str, "EXIT"))
		return EXIT;
	return ERROR_OP_DESCONOCIDA;
}

long int calcular_tamanio_archivo(FILE * archivo){
	fseek(archivo, 0, SEEK_END);
	long int tamanio_archivo = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	return tamanio_archivo;
}

void agregar_instrucciones(t_list * lista_ins, char** instrucciones){
    void _f_aux(char *elem ){
        uint32_t numero_de_veces;
        numero_de_veces = agregar_una_instruccion(lista_ins, elem, 0);	//DETALLE!! funcion para trabajar ya que las commons toman solo un parametro creo la auxiliar para pasar los dos que necesito a la funcion que realiza la logica
        for(uint32_t i = 0; i < numero_de_veces; i++){					//Acá entra sólo si es una NO_OP. Si es NO_OP 5, entrara 5 veces para realizar NO_OP
            agregar_una_instruccion(lista_ins, elem, 1);
        }
        free(elem);
    }
    string_iterate_lines(instrucciones, _f_aux);
}


uint32_t agregar_una_instruccion(t_list * lista_ins, void * param, uint32_t flag){
	char * inst = (char *) param;
	instruccion *instruccion_aux =  malloc(sizeof(instruccion));
	instruccion_aux->tam_param =0;
	instruccion_aux->cod_op = 0;
	instruccion_aux->parametros = NULL;
	char **instrucciones = string_split(inst, " ");


	instruccion_aux->cod_op = convertir_instruccion(instrucciones[0]);
	free(instrucciones[0]);

	if(instruccion_aux->cod_op == NO_OP && !flag){
		uint32_t numero_de_veces = (uint32_t)atoi(instrucciones[1]);
		free(instrucciones[1]);
		free(instruccion_aux);
		free(instrucciones);
		return numero_de_veces;
	}
	int j = 1;

	if(instruccion_aux->cod_op == NO_OP && flag){
		instruccion_aux->tam_param = 0;
		while(instrucciones[j]){
			free(instrucciones[j]);
			j++;
		}
	}
	j =1;

	while(instrucciones[j] && !flag){
		uint32_t numero = (uint32_t)atoi(instrucciones[j]);
		instruccion_aux->tam_param += sizeof(uint32_t);
		instruccion_aux->parametros = (uint32_t *)realloc(instruccion_aux->parametros, j*sizeof(uint32_t));
		instruccion_aux->parametros[j-1] = numero;
		free(instrucciones[j]);

		j++;
	}
	list_add(lista_ins, instruccion_aux);
	free(instrucciones);
	return 0;
}

int main(int argc, char** argv){
    // Consola va a recibir como parámetro del main 2 path:
    //  1: Archivo de configuración. consola.config (Por ahora lo dejamos en la carpeta consola, pero se manda por parámetro la ruta así que puede ser cualquiera)
    //  2: Archivo de pseudocódigo con las instrucciones a ejecutar.

    /*
    1:
    En el archivo de configuracion vamos a tener la IP y el Puerto del Kernel.
    Con esos datos ya basta para establecer la conexión con el mismo.
    crear_conexion(logger, nombre, ip, puerto)
    En nuestro caso sería
    crear_conexion(logger, "Kernel", kernelIP, kernelPuerto);
    Esto lo guardamos en una variable ya que si salió todo bien devuelve un socket. Caso contrario, devuelve un 0.
    */

    /*
    2:
    Una vez que la conexión con el kernel ya está establecida la idea es mandarle las instrucciones del archivo (2)
    Para esto tendríamos que preparar un "paquete" con las instrucciones y una vez terminado, enviarlo.
       - Crear paquete: Tenemos que parsear cada línea terminada con "/n" como una instrucción y generar el listado de instrucciones.
    Luego de haberlo enviado la consola quedará a la espera de un mensaje del Kernel que indique que finalizó el proceso.
    */


    t_log *logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);
    t_config *config = config_create("consola.config");
    // Config Tiene IP y Puerto. En realidad cuando se mande por parámetro del main la ruta va a ser diferente, por ahora es así.

    
    // CLIENTE -> Kernel

    // Conexion con Kernel
    //int conexion = conectar_con(KERNEL, config, logger);


    // PARSER

    // LA IDEA ES GENERAR UNA LISTA DONDE VAYAMOS METIENDO TODAS LAS INSTRUCCIONES, Y DESPUES MANDAMOS ESA LISTA AL KERNEL                                                         
    // NECESITAMOS SEPARAR LA INSTRUCCION DE LOS PARAMETROS                                                                                                                        
    // PODEMOS HACER UN STRUCT QUE TENGA COMO INFORMACION LA OPERACION A REALIZAR Y LOS PARAMETROS (Y SU TAMANIO)                                                                  

    // HACEMOS UN PRIMER SPLIT DE LAS INSTRUCCIONES HASTA EL BARRA N                                                                                                               
    // DESPUES LEEMOS UNA LINEA Y HACEMOS OTRO SPLIT POR ESPACIOS PARA SEPARAR LAS INSTRUCCIONES DE SUS PARAMETROS 

    // Struct:
    //    String instruccion
    //    t_list* parametros

    // Por consola te llega un path con el archivo con instrucciones.txt
    // Lees archivo 
    // Es un solo send al kernel

    // op_code_paquete, 

    t_list* lista_instrucciones = list_create();                                        // Creamos una lista donde se colcaran las instrucciones                      

    char* path_instrucciones = "ejemplo.txt";

    FILE* archivo_instrucciones = fopen(path_instrucciones, "r");                       // Abrimos el archivo de pseudocodigo con las instrucciones                   

    if(archivo_instrucciones == NULL) {
        log_error(logger, "NO SE PUDO ABRIR EL ARCHIVO DE INSTRUCCIONES");              // Verificamos que no haya error                                              
    }
    else {
        log_info(logger,"se pudo abrir :D");
    }


    long int tamanio_archivo = calcular_tamanio_archivo(archivo_instrucciones);              // Calculamos el tamanio del archivo   

    //log_info(logger, "%d", tamanio_archivo);                                       

    char* archivo_string = malloc(tamanio_archivo + 1);                                 // Reservamos memoria donde se va a guardar lo leido                          

    // guardamos en archivo_string lo leido
    fread(archivo_string, tamanio_archivo, 1, archivo_instrucciones);

    if(archivo_string == NULL){
        log_error(logger, "NO SE PUDO LEER EL ARCHIVO");
    }

    archivo_string[tamanio_archivo] = '\0';                                             // Agregamos el EOF para que el fread sepa hasta donde leer el archivo  

    printf(archivo_string, "%s");
    printf("\n\n\n\n");      

    fclose(archivo_instrucciones);

    char** instrucciones = string_split(archivo_string, '\n');                          // Separamos las funciones de acuerdo a un caracter centinela, en este caso e\l \n.                                                                                                                                                                 

    /*
    Si tengo el archivo
        SET AX HOLA
        MOV_OUT 120 AX
        WAIT DISCO
    Ejemplo para Archivo:
        ["SET AX HOLA", "MOV_OUT 120 AX", "WAIT DISCO"]

    */

    agregar_instrucciones(lista_instrucciones, instrucciones);                          // Agregamos las instrucciones separadas a la lista                           


   
    /*
    


    free(instrucciones);
    

    

    








    
    // ENVIO DE MENSAJES

    int numero;
    printf("Ingrese un numero: ");
    scanf("%d", &numero);
    send_numero(conexion, numero);

    config_destroy(config);
    log_destroy(logger);
    */
}




/*

enum {
    MOVE,
    FLOPA
} cod_op_instruccion

typedef struct {
    cod_op_instruccion operacion
    int tam_parametros
    parametro* parametros
} instruccion

typedef struct {
    parametro
    tamanioParametro
} parametro

"SET"AX","BX"

SET       ["AX HOLA"]
["SET","AX","HOLA"] -> Volas el primero y lo reemplazas por el opCode
["AX","HOLA"]

MOV_OUT 120 AX

WAIT DISCO

-> instrucciones 
-> splitiamos por espacios
-> vamos a tener otra lista de char* con la instruccion y parametros ["SET","AX ","HOLA"]
-> agarramos el primer elemento para tener la instruccion y meterla en la listaIns
        instruccion->operacion
-> seguimos iterando la lista de char* para conseguir el siguiente parametro y agregarlo a listaIns
-> Repito el paso anterior hasta que termine lista char* */


// Qué es lo que le mandamos al kernel?
// listaIns