#include "memoria.h"

int main(int argc, char** argv){
    logger = log_create("memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);

    if (argc==1){
        config = config_create("memoria.config");
    }
    else if (argc==2) {
        config = config_create(argv[1]); // Un ejemplo seria ../tests/BASE/memoria.config
    }
    else{
        log_error(logger, "Cantidad de parametros incorrecta. Debería ser ./memoria PATH_CONFIG \n"); 
        return EXIT_FAILURE;
    }

    cargar_config_memoria(config);
    


    // SERVER -> CPU, Kernel, FileSystem
    int server_fd = preparar_servidor("MEMORIA", config, logger);

    //cliente_filesystem = esperar_cliente(server_fd, logger, "Memoria");
    //cliente_cpu = esperar_cliente(server_fd, logger, "Memoria");
    cliente_kernel = esperar_cliente(server_fd, logger, "Memoria");
    
    t_segmento* segmento_cero = malloc(sizeof(t_segmento));
    segmento_cero->id_segmento = 0;
    segmento_cero->direccion_base_segmento = 0; // dudosito
    segmento_cero->tamanio_segmento = config_memoria.TAM_SEGMENTO_0;

    //while(1) {
        int solicitud;

        RECV_INT(cliente_kernel, solicitud);
        if(solicitud == 1) 
        {
            log_info(logger,"RECIBI UNA SOLICITUD DE SEGMENTOS");
            
            t_list* tabla_segmentos = list_create();
            list_add(tabla_segmentos, segmento_cero);

            for(int i=1; i < config_memoria.CANT_SEGMENTOS; i++) {
                
                t_segmento* segmento = malloc(sizeof(t_segmento));
                segmento->id_segmento = i;
                segmento->direccion_base_segmento = 0; // dudosito
                segmento->tamanio_segmento = 10; // dudosito -> no sabemos como definirlo
                
                list_add(tabla_segmentos, segmento);
            }
            //send_segmentos(cliente_kernel, tabla_segmentos);
            
            
            //creamos la tabla de segmentos y se la mandamos a kernel
        }
    //}
    




    // aca crea el hilo de "responder_orden()"

    liberar_conexion(&server_fd);
    cerrar_programa(logger,config);

    return 0;
}