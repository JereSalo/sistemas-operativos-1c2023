#include "consola.h"


int main(int argc, char** argv){
    logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

    char* path_instrucciones;
    // ./consola PATH_CONFIG PATH_INSTRUCTIONS - Hacer make antes !!
    // Para uso cotidiano seguimos usando ./exec, si queremos probar unas instrucciones en particular usamos el comando de arriba para ejecutar.
    if (argc==1){
        config = config_create("consola.config");
        path_instrucciones = "instrucciones/fs_write.txt";
    }
    else if (argc==3) {
        config = config_create(argv[2]);
        path_instrucciones = argv[1];
    }
    else{
        log_error(logger, "Cantidad de parametros incorrecta. Debería ser ./consola PATH_CONFIG PATH_INSTRUCCIONES\n"); 
        return EXIT_FAILURE;
    }
    
    server_kernel = conectar_con(KERNEL, config, logger);

    t_list* instrucciones = generar_lista_instrucciones(path_instrucciones);
    
    send_instrucciones(server_kernel, instrucciones);

    // Confirmacion de recepcion
    int confirmacion;
    RECV_INT(server_kernel, confirmacion);
    if(confirmacion)
        log_info(logger, "Confirmacion Recibida :D - Kernel recibio las instrucciones\n");
    else{
        log_error(logger, "Se rompio todo :(\n");
        return EXIT_FAILURE;
    }
    
    // Esperar mensaje que indique finalizacion del proceso.
    int pid;
    RECV_INT(server_kernel, pid);

    char mensaje_finalizacion[50];
    recv_string(server_kernel, mensaje_finalizacion);
    
    log_info(logger, "Proceso %d ha finalizado con motivo: %s", pid, mensaje_finalizacion);
    
    list_destroy_and_destroy_elements(instrucciones,free);
    cerrar_programa(logger,config);

    return EXIT_SUCCESS;
}

