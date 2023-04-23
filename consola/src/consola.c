#include "consola.h"


int main(int argc, char** argv){
    // Consola va a recibir como parámetro del main 2 path: El config y las instrucciones
    
    t_log *logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

    /*
    if(argc != 2) {
        log_error(logger, "CONSOLA RECIBIO UNA CANTIDAD INCORRECTA DE PARAMETROS");
    }
    */

    t_config *config = config_create("consola.config");                         //cambiar por ruta recibida del main
    
    
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


    
    //VAMOS A HACERLA CORTA
    
    
    char* path_instrucciones = strdup("ejemplo.txt");

    // Leemos el archivo y devolvemos un string con el contenido del archivo para trabajarlo mas comodamente
    char* archivo_string = leer_archivo(path_instrucciones, logger);


    // Spliteamos cada linea del string para obtener "INSTRUCCION PARAM ... ..."
    char** instrucciones = string_split(archivo_string, "\n");
    free(archivo_string);

    //printf("%s", instrucciones[0]);

    // Ahora estamos en condiciones de serializar y mandar esto a Kernel
    // Despues Kernel se lo manda a CPU y que CPU se encargue de hacer el nuevo split y que el separe los parametros
    /* Ni siquiera es necesario usar listas ya que el CPU despues de hacer el nuevo split
        tendra un array con una instruccion tipo ["MOV", "AX", "HOLA"] y despues lo que hacemos es 
        tomar al primer elemento para despues hacer un switch y preguntar que instruccion tenemos y 
        segun el CASE, recibiremos los parametros que sean necesarios.
    */

    

    free(path_instrucciones);

    //send_instrucciones(conexion, instrucciones);

    string_array_destroy(instrucciones);

    cerrar_programa(logger,config);
}

