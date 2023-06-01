#include "consola.h"


int main(int argc, char** argv){
    // Consola va a recibir como parámetro del main 2 path: El config y las instrucciones
    
    logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

    char* path_instrucciones;
    // ./consola PATH_CONFIG PATH_INSTRUCTIONS - Hacer make antes !!
    // ./consola "consola.config" "ejemplo3.txt"
    // Para uso cotidiano seguimos usando ./exec, si queremos probar unas instrucciones en particular usamos el comando de arriba para ejecutar.
    if (argc==1){
        config = config_create("consola.config");
        path_instrucciones = "ejemplo.txt";
    }
    else if (argc==3) {
        config = config_create(argv[1]);
        path_instrucciones = argv[2];
    }
    else{
        log_error(logger, "Cantidad de parametros incorrecta. Debería ser ./consola PATH_CONFIG PATH_INSTRUCCIONES"); 
        return EXIT_FAILURE;
    }
    
    server_kernel = conectar_con(KERNEL, config, logger);

    t_list* instrucciones = generar_lista_instrucciones(path_instrucciones);
    
    send_instrucciones(server_kernel, instrucciones);

    // Confirmacion de recepcion
    char confirmacion_recepcion[50];
    recv_string(server_kernel, confirmacion_recepcion);

    log_info(logger, "%s", confirmacion_recepcion);
    // Esperar mensaje que indique finalizacion del proceso.
    char mensaje_finalizacion[50];
    recv_string(server_kernel, mensaje_finalizacion);

    log_info(logger, "Proceso ha finalizado con motivo: %s", mensaje_finalizacion);
    
    list_destroy_and_destroy_elements(instrucciones,free);
    cerrar_programa(logger,config);
}

