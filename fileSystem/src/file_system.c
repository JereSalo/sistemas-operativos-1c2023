#include "file_system.h"

int main(int argc, char** argv){
    logger = log_create("filesystem.log", "FILESYSTEM", true, LOG_LEVEL_INFO);

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

    // Ojo: El path del config es absoluto, si no tenes la carpeta 'fs' creada entonces el fopen() va a tirar error.
    //archivo_bitmap = abrir_archivo_bitmap();
    //archivo_superbloque = abrir_archivo_superbloque();
    //archivo_bloques = abrir_archivo_bloques();

    //if(archivo_superbloque == NULL || archivo_bitmap == NULL || archivo_bloques == NULL)
    //    return EXIT_FAILURE;


    // CLIENTE -> Memoria
    server_memoria = conectar_con(MEMORIA, config, logger);
    
    //HACER HANDSHAKE

    // Levantamos el superbloque
    archivo_superbloque = config_create("superbloque.dat");

    cargar_info_superbloque(archivo_superbloque);

    log_info(logger, "Info del superbloque cargada - BLOCK_SIZE: %d   BLOCK_COUNT: %d \n", info_superbloque.BLOCK_SIZE, info_superbloque.BLOCK_COUNT);

    
    // Levantamos el bitmap de bloques -> si no existe, lo creamos -> revisar como lo acoplamos con el bitarray
    size_t tamanio_bitmap;
    levantar_archivo(config_filesystem.PATH_BITMAP, &archivo_bitmap, &tamanio_bitmap , "bitmap");
    
    
    
    //mapear_archivo(fd, tamanio_archivo, tipo_archivo); -> el tipo lo agrego porque si es bitmap tenemos que acoplarlo con el bitarray

    // Idem con el archivo de bloques
    size_t tamanio_archivo_bloques;
    levantar_archivo(config_filesystem.PATH_BLOQUES, &archivo_bloques, &tamanio_archivo_bloques , "bloques");
    //lol
    
    
    //mapear_archivo();

    // Entre ellas estan lista_fcbs
    //crear_estructuras_administrativas();

    
    
    
    // SERVER
    int server_fd = preparar_servidor("FILESYSTEM", config, logger);

    // Aca ya no mas esperar_clientes, esperamos individualmente.
    cliente_kernel = esperar_cliente(server_fd, logger, "fileSystem");


    //procesar_kernel_filesystem();


    liberar_conexion(&server_fd);
    cerrar_programa(logger,config);

    return 0;
}