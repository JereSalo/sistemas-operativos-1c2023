#include "consola.h"


int main(int argc, char** argv){
    // Consola va a recibir como parámetro del main 2 path: El config y las instrucciones
    
    t_log *logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

    t_config *config = config_create("consola.config");                         //cambiar por ruta recibida del main
    
    
    // CLIENTE -> Kernel

    // Conexion con Kernel
    int conexion = conectar_con(KERNEL, config, logger);
    
    
    char* path_instrucciones = strdup("ejemplo.txt");

    t_list* instrucciones = generar_lista_instrucciones(path_instrucciones);    

    
    send_instrucciones(conexion, instrucciones);

    
    free(path_instrucciones);
    list_destroy_and_destroy_elements(instrucciones,free);
    cerrar_programa(logger,config);
}

