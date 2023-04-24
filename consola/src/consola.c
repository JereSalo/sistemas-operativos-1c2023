#include "consola.h"


int main(int argc, char** argv){
    // Consola va a recibir como parámetro del main 2 path: El config y las instrucciones
    
    t_log *logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

    t_config *config = config_create("consola.config");                         //cambiar por ruta recibida del main
    
    
    // CLIENTE -> Kernel

    // Conexion con Kernel
    int conexion = conectar_con(KERNEL, config, logger);


    
    
    char* path_instrucciones = strdup("ejemplo.txt");

    // Leemos el archivo y devolvemos un string con el contenido del archivo para trabajarlo mas comodamente
    char* archivo_string = leer_archivo(path_instrucciones, logger);

    t_list* instrucciones = generar_lista_instrucciones(archivo_string);

                
    
    
    // size_t size_generico; // Este size es de prueba para testear serializar, en realidad está en el send.
    
    

    send_instrucciones(conexion, instrucciones);
   


    free(path_instrucciones);
    free(archivo_string);
    list_destroy(instrucciones);
    cerrar_programa(logger,config);

}

