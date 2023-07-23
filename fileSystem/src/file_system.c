#include "file_system.h"


void sigint_handler(int signum) {
    // Acciones a realizar al recibir la señal SIGINT
    // Hay mas cosas para hacer pero estas son algunas por ahora
    
    mostrar_bitarray();
    mostrar_contenido_archivo("bloques.dat");
    mostrar_punteros_archivo_bloques();
    //mostrar_archivo_mapeado_bloques();

    desmapear_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);
    desmapear_archivo(archivo_bloques_mapeado, tamanio_archivo_bloques);

    exit(0);
}



int main(int argc, char** argv){
    
    signal(SIGINT, sigint_handler);
    logger = log_create("filesystem.log", "FILESYSTEM", true, LOG_LEVEL_DEBUG);

    if (argc==1){
        config = config_create("filesystem.config");
    }
    else if (argc==2) {
        config = config_create(argv[1]); // Un ejemplo seria ../tests/BASE/filesystem.config
    }
    else{
        log_error(logger, "Cantidad de parametros incorrecta. Debería ser ./filesystem PATH_CONFIG \n"); 
        return EXIT_FAILURE;
    }

    cargar_config_filesystem(config);

    
    // CLIENTE -> Memoria
    server_memoria = conectar_con(MEMORIA, config, logger);
    
    //HACER HANDSHAKE

    // Levantamos el superbloque
    archivo_superbloque = config_create("superbloque.dat");

    cargar_info_superbloque(archivo_superbloque);

        
    // Levantamos el bitmap de bloques
    levantar_archivo(config_filesystem.PATH_BITMAP, &archivo_bitmap, &tamanio_archivo_bitmap , "bitmap");
    
    mapear_archivo("bitmap");

    mostrar_bitarray();


    //Se supone que cuando modificamos el array se esta modificando el espacio de memoria tambien
    //Asi que cuando sincronizamos con el archivo deberia ser lo mismo

    //bitarray_set_bit(bitarray_bloques, 0);

    //mostrar_bitarray();

    //sincronizar_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);

    //desmapear_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);
    

    // Para chequear que este todo OK volvemos a mapear el archivo y volvemos a mostrar el bitarray
    //mapear_archivo("bitmap");
    //mostrar_bitarray();


    // Idem con el archivo de bloques
    levantar_archivo(config_filesystem.PATH_BLOQUES, &archivo_bloques, &tamanio_archivo_bloques , "bloques");
    
    mapear_archivo("bloques");

    // Accedemos al archivo mapeado y lo manejamos como un char*
    //char* data_as_chars = (char*)archivo_bloques_mapeado;
    //for (int i = 0; i < tamanio_archivo_bloques; i++) {
    //    data_as_chars[i] = 'C'; // You can modify the data here as needed
    //}


    //log_debug(logger, "Mostrando data del archivo mapeado en memoria: %s \n", data_as_chars);

    //sincronizar_archivo(archivo_bloques_mapeado, tamanio_archivo_bloques);

    //desmapear_archivo(archivo_bloques_mapeado, tamanio_archivo_bloques);


    //Ahora quiero leer el archivo para chequear que este todo OK
    //mostrar_contenido_archivo("bloques.dat");
    
    
    // Entre ellas estan lista_fcbs
    crear_estructuras_administrativas();

    
    // SERVER
    int server_fd = preparar_servidor("FILESYSTEM", config, logger);

    // Aca ya no mas esperar_clientes, esperamos individualmente.
    cliente_kernel = esperar_cliente(server_fd, logger, "fileSystem");


    procesar_kernel_filesystem();


    


    liberar_conexion(&server_fd);
    cerrar_programa(logger,config);

    return 0;
}