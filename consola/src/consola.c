#include "consola.h"


int main(int argc, char** argv){
    // Consola va a recibir como parámetro del main 2 path: El config y las instrucciones
    
    


    t_log *logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);
    t_config *config = config_create("consola.config");
    // Config Tiene IP y Puerto. En realidad cuando se mande por parámetro del main la ruta va a ser diferente, por ahora es así.

    
    // CLIENTE -> Kernel

    // Conexion con Kernel
    //int conexion = conectar_con(KERNEL, config, logger);


    /*
    Archivo de Pseudocódigo con instrucciones a ejecutar
    
    Una vez que la conexión con el kernel ya está establecida la idea es mandarle las instrucciones del archivo (2)
    Para esto tendríamos que preparar un "paquete" con las instrucciones y una vez terminado, enviarlo.
       - Crear paquete: Tenemos que parsear cada línea terminada con "/n" como una instrucción y generar el listado de instrucciones.
    Luego de haberlo enviado la consola quedará a la espera de un mensaje del Kernel que indique que finalizó el proceso.
    */

    // PARSER

    // LA IDEA ES GENERAR UNA LISTA DONDE VAYAMOS METIENDO TODAS LAS INSTRUCCIONES, Y DESPUES MANDAMOS ESA LISTA AL KERNEL                                                         
    // NECESITAMOS SEPARAR LA INSTRUCCION DE LOS PARAMETROS                                                                                                                        
    // PODEMOS HACER UN STRUCT QUE TENGA COMO INFORMACION LA OPERACION A REALIZAR Y LOS PARAMETROS (Y SU TAMANIO)                                                                  

    // HACEMOS UN PRIMER SPLIT DE LAS INSTRUCCIONES HASTA EL BARRA N                                                                                                               
    // DESPUES LEEMOS UNA LINEA Y HACEMOS OTRO SPLIT POR ESPACIOS PARA SEPARAR LAS INSTRUCCIONES DE SUS PARAMETROS 

    // Struct:
    //    String operacion
    //    t_list* parametros

    // Por consola te llega un path con el archivo con instrucciones.txt
    // Lees archivo 
    // Es un solo send al kernel

    // op_code_paquete, 

    //Llamamos a una funcion que se encargue de parsear

    char* path_instrucciones = "ejemplo.txt";
    t_list* lista_instrucciones = list_create();                                        // Creamos una lista donde se colcaran las instrucciones
    
    parsear_instrucciones(path_instrucciones, lista_instrucciones,logger);

    free(path_instrucciones);

    // se supone que despues vamos a tener una hermosa lista de instrucciones con sus respectivos parametros y vamos a serializar 

    // lista_instrucciones = [ins1 [param1, param2], ins2 [param]]
/*
struct{
    operacion
    t_list* parame
}instruccion;
*/
    
    /*
    Si tengo el archivo
        SET AX HOLA
        MOV_OUT 120 AX
        WAIT DISCO
    Ejemplo para Archivo:
        ["SET AX HOLA", "MOV_OUT 120 AX", "WAIT DISCO"]

    */

                               

   
    /*

     
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


-> archivo
-> splitiamos con \n
-> instrucciones 
-> splitiamos por espacios
-> vamos a tener otra lista de char* con la instruccion y parametros ["SET","AX ","HOLA"]
-> agarramos el primer elemento para tener la instruccion y meterla en la listaIns
        instruccion->operacion
-> seguimos iterando la lista de char* para conseguir el siguiente parametro y agregarlo a listaIns
-> Repito el paso anterior hasta que termine lista char* */


// Qué es lo que le mandamos al kernel?
// listaIns


/*
-> tengo instrucciones 
-> tengo que separar instrucciones de parametros 
-> tengo que iterar 

 lista ins = ["COD P P P", "COD P", "COD P P"]
 
 for(i=0, i<ins.size(), i++){
    // aca trabajamos individualmente con cada instruccion ?
 }

list_iterate(lista_instrucciones, _f_aux);	// Cada instruccion en la lista de instrucciones va a realziar la funcion auxiliar
La funcion aux lo que podria hacer es armar la lista en donde cada elemento es una instruccion con la lista de param

instrucciones[0] = "instrucioj parametro parametro";
instrucciones[1] = "instrucion parametro";

"instrucioj parametro parametro",
"instrucion parametro"
*/