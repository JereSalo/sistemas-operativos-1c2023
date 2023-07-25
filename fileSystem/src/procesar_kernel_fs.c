#include "procesar_kernel_fs.h"

void procesar_kernel_filesystem(){
    while(1) {
        op_code cod_op = recibir_operacion(cliente_kernel);

        switch((int)cod_op) {
            case SOLICITUD_LECTURA_DISCO:
            {
                // Leer informacion de disco y escribirla en memoria
                
                char nombre_archivo[64];
                recv_string(cliente_kernel, nombre_archivo);

                int cantidad_bytes;
                int direccion_fisica;
                int pid;
                int puntero;
                
                RECV_INT(cliente_kernel, direccion_fisica);
                RECV_INT(cliente_kernel, cantidad_bytes);
                RECV_INT(cliente_kernel, pid);
                RECV_INT(cliente_kernel, puntero);

                log_warning(logger, "Leer Archivo: %s - Puntero: %d - Memoria: %d - Tamanio: %d \n", nombre_archivo, puntero, direccion_fisica, cantidad_bytes); //LOG LECTURA ARCHIVO

                // Aca vamos a ir guardando la informacion que vayamos leyendo 
                char* informacion_leida = malloc(cantidad_bytes + 1);
                
                // Buscamos el FCB del archivo en cuestion
                t_fcb* archivo = buscar_archivo_en_lista_fcbs(nombre_archivo);

                // Averiguamos cuantos bloques debemos leer
                int cant_bloques_a_leer = ceil((float)cantidad_bytes / info_superbloque.BLOCK_SIZE);

                log_debug(logger, "Tengo que leer %d bloques \n", cant_bloques_a_leer);

                // Tenemos que hacer un algoritmo que busque el proximo bloque a leer en el FS
                // Este algoritmo lo que haria seria algo asi como buscarte el bloque del FS correspondiente al puntero

                uint32_t bloque_a_leer;
                bool acceso_bloque_punteros = 0;

                for( ; cant_bloques_a_leer > 0; cant_bloques_a_leer--) {
                    
                    bloque_a_leer = buscar_bloque(archivo, puntero, &acceso_bloque_punteros, nombre_archivo);

                    if(cantidad_bytes > info_superbloque.BLOCK_SIZE) {
                        // Leemos lo maximo que se puede y le restamos para leer menos en el proximo bloque
                        leer_bloque(bloque_a_leer, informacion_leida, info_superbloque.BLOCK_SIZE);
                        usleep(config_filesystem.RETARDO_ACCESO_BLOQUE * 1000); 
                        cantidad_bytes -= info_superbloque.BLOCK_SIZE;
                        puntero += info_superbloque.BLOCK_SIZE;
                    }
                    else {
                        leer_bloque(bloque_a_leer, informacion_leida, cantidad_bytes);
                        usleep(config_filesystem.RETARDO_ACCESO_BLOQUE * 1000); 
                        puntero += cantidad_bytes;
                    }
                }
                
                // Agregamos el \0 
                informacion_leida[cantidad_bytes] = '\0';
                

                log_debug(logger, "MANDO PETICION ESCRITURA A MEMORIA");
                // Mandamos a memoria para que los escriba en su espacio
                send_peticion_escritura(server_memoria, direccion_fisica, cantidad_bytes, informacion_leida);
                SEND_INT(server_memoria, pid);//CHEQUEAR

                free(informacion_leida);

                // Esperamos que memoria nos avise cuando termine de escribir
                char confirmacion[5];
                recv_string(server_memoria, confirmacion);

                if(string_equals_ignore_case(confirmacion, "OK")) 
                {
                    log_info(logger, "Lectura de archivo exitosa \n");
                }

                // Le mandamos confirmacion al Kernel y el PID para que desbloquee el proceso
                send_opcode(cliente_kernel, RESPUESTA_FREAD);
                SEND_INT(cliente_kernel, 1);
                SEND_INT(cliente_kernel, pid);

                break;
            }
            case SOLICITUD_ESCRITURA_DISCO:
            {
                // Leer informacion de memoria y escribirla en disco
                
                char nombre_archivo[64];
                recv_string(cliente_kernel, nombre_archivo);
                
                int cantidad_bytes;
                int direccion_fisica;
                int pid;
                int puntero;

                RECV_INT(cliente_kernel, direccion_fisica);
                RECV_INT(cliente_kernel, cantidad_bytes);
                RECV_INT(cliente_kernel, pid);
                RECV_INT(cliente_kernel, puntero);

                log_warning(logger, "Escribir Archivo: %s - Puntero: %d - Memoria: %d - Tamanio: %d \n", nombre_archivo, puntero, direccion_fisica, cantidad_bytes); //LOG ESCRITURA ARCHIVO
                
                // Leemos de memoria
                send_peticion_lectura(server_memoria, direccion_fisica, cantidad_bytes);
                SEND_INT(server_memoria, pid);//CHEQUEAR
                
                // Recibimos los datos leidos desde memoria
                char valor_a_escribir[64];
                recv_string(server_memoria, valor_a_escribir);
 
                // Buscamos el FCB del archivo en cuestion
                t_fcb* archivo = buscar_archivo_en_lista_fcbs(nombre_archivo);

                // Averiguamos cuantos bloques debemos escribir
                int cant_bloques_a_escribir = ceil((float)cantidad_bytes / info_superbloque.BLOCK_SIZE);
                
                log_debug(logger, "Tengo que escribir %d bloques \n", cant_bloques_a_escribir);

                // Tenemos que hacer un algoritmo que busque el proximo bloque a escribir en el FS
                // Este algoritmo lo que haria seria algo asi como buscarte el bloque del FS correspondiente al puntero

                uint32_t bloque_a_escribir;
                int offset = 0;
                bool acceso_bloque_punteros = 0;
                
                for( ; cant_bloques_a_escribir > 0; cant_bloques_a_escribir--) {
                    
                    bloque_a_escribir = buscar_bloque(archivo, puntero, &acceso_bloque_punteros, nombre_archivo);

                    if(cantidad_bytes > info_superbloque.BLOCK_SIZE) {
                        // Escribimos lo maximo que se puede y le restamos para escribir menos en el proximo bloque
                        escribir_bloque(bloque_a_escribir, valor_a_escribir + offset, info_superbloque.BLOCK_SIZE, offset);
                        usleep(config_filesystem.RETARDO_ACCESO_BLOQUE * 1000); 
                        cantidad_bytes -= info_superbloque.BLOCK_SIZE;
                        puntero += info_superbloque.BLOCK_SIZE;
                        offset+=info_superbloque.BLOCK_SIZE;

                    }
                    else {
                        escribir_bloque(bloque_a_escribir, valor_a_escribir, cantidad_bytes, offset);
                        usleep(config_filesystem.RETARDO_ACCESO_BLOQUE * 1000); 
                        puntero += cantidad_bytes;
                        offset+=cantidad_bytes;
                    }

                }
                            
                // MESSI
                log_info(logger, "Datos escritos en disco: %s \n", valor_a_escribir);

                mostrar_contenido_archivo("bloques.dat");
                
                // Mandamos confirmacion de finalizacion de operacion al Kernel y el PID para que desbloquee al proceso
                send_opcode(cliente_kernel, RESPUESTA_FWRITE);
                SEND_INT(cliente_kernel, 1);
                SEND_INT(cliente_kernel, pid);
                
                break;
            }
            case SOLICITUD_CREAR_ARCHIVO:
            {
                char nombre_archivo[64];
                recv_string(cliente_kernel, nombre_archivo);
                
                log_warning(logger, "Crear archivo: %s \n", nombre_archivo); //LOG CREAR ARCHIVO

                t_fcb* archivo = malloc(sizeof(t_fcb));
                archivo->nombre = strdup(nombre_archivo);
                archivo->tamanio = 0;
                archivo->puntero_directo = -1;
                archivo->puntero_indirecto = -1;

                list_add(lista_fcbs, archivo);


                log_debug(logger, "Voy a crear la entrada de directorio");
                crear_entrada_directorio(nombre_archivo);

                send_opcode(cliente_kernel, RESPUESTA_CREATE);

                break;
            }
            case SOLICITUD_ABRIR_ARCHIVO:
            {
                char nombre_archivo[64];
                recv_string(cliente_kernel, nombre_archivo);

                log_debug(logger, "Nombre archivo recibido: %s", nombre_archivo);

                // Verifica si existe el archivo -> buscamos en la lista de fcbs
                t_fcb* archivo = buscar_archivo_en_lista_fcbs(nombre_archivo);

                log_warning(logger, "Abrir archivo: %s \n", nombre_archivo); //LOG APERTURA DE ARCHIVO

                if(archivo == NULL) {
                    log_debug(logger, "El archivo no existe \n"); 
                    send_opcode(cliente_kernel, RESPUESTA_FOPEN);
                    SEND_INT(cliente_kernel, 1);
                    send_string(cliente_kernel, nombre_archivo);
                }
                else {
                    log_debug(logger, "El archivo existe, no es necesario crearlo \n");
                    send_opcode(cliente_kernel, RESPUESTA_FOPEN);
                    SEND_INT(cliente_kernel, 0);
                    send_string(cliente_kernel, nombre_archivo);
                }

                log_debug(logger, "Envie respuesta a Kernel"); 

                break;
            }
            case SOLICITUD_TRUNCAR_ARCHIVO:
            {
                
                char nombre_archivo[64];
                int tamanio;
                int pid;

                recv_string(cliente_kernel, nombre_archivo);
                RECV_INT(cliente_kernel, tamanio);
                RECV_INT(cliente_kernel, pid);      // Para despues mandarle al Kernel que proceso se debe desbloquear 
            
                log_warning(logger, "Truncar archivo: %s - Tamanio: %d \n", nombre_archivo, tamanio); //LOG TRUNCATE ARCHIVO
                
                t_fcb* archivo = buscar_archivo_en_lista_fcbs(nombre_archivo);

                
                if(archivo->tamanio == tamanio) {
                    log_info(logger, "No se truncara ya que el archivo tiene dicho tamanio \n");
                }
                else if(archivo->tamanio > tamanio) {
                    achicar_archivo(archivo, tamanio);
                }
                else if(archivo->tamanio < tamanio) {
                    agrandar_archivo(archivo, tamanio);
                }
                
                // Debug
                mostrar_bitarray();
                mostrar_punteros_archivo_bloques();

                // Mandamos el PID al Kernel para que desbloquee al proceso
                send_opcode(cliente_kernel, RESPUESTA_FTRUNCATE);
                SEND_INT(cliente_kernel, pid);

                break;
            }
        }
    }
}


void achicar_archivo(t_fcb* archivo, int tamanio_nuevo) {

    char file_path[4096];

    // Concatena en una variable e imprime dicha variable
    snprintf(file_path, 4096, "fcb/%s.dat", archivo->nombre);

    t_config* fcb_archivo = config_create(file_path);

    // Obtenemos la cantidad de bloques que tiene el archivo actualmente
    int cant_bloques_actual = ceil( (float)(archivo->tamanio) / info_superbloque.BLOCK_SIZE);

    // Obtenemos la cantidad de bloques de datos que va a tener el archivo truncado
    int cant_bloques_nuevos = ceil( (float)tamanio_nuevo / info_superbloque.BLOCK_SIZE);

    int bloques_a_quitar = cant_bloques_actual - cant_bloques_nuevos;

    // Si el archivo no tiene PI, entonces simplemente cambiamos el tamanio del mismo y ya 
    // En todos los casos cambiamos el tamanio en la entrada de directorio y en el FCB en memoria

    uint32_t* bloque_indirecto = (uint32_t*)archivo_bloques_mapeado;

    // Si lo truncamos a 0 es el unico caso donde debemos desasignar del bitarray el PD (y ademas se puede llegar a desasignar del PI)
    
    if(tamanio_nuevo == 0) {

        int bloque_directo = archivo->puntero_directo;

        log_warning(logger, "Acceso a Bitmap - Bloque: %d, Estado: %d \n", bloque_directo, bitarray_test_bit(bitarray_bloques, bloque_directo)); //LOG ACCESO A BITMAP        // Sacamos al puntero directo del bitmap
        bitarray_clean_bit(bitarray_bloques, bloque_directo);
        sincronizar_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);

        bloques_a_quitar--;
    }

    // Si el archivo tiene PI debemos recorrer cada PD del PI (de reversa mami) e ir marcando como libre cada bloque en el bitarray
    if(archivo->puntero_indirecto != -1) {    
        
        // Calculamos la posicion donde se encuentra el ultimo PD del PI 
        int posicion_ultimo_puntero = calcular_posicion_ultimo_puntero(archivo);
        int posicion_bloque_punteros = (archivo->puntero_indirecto * info_superbloque.BLOCK_SIZE);

        log_debug(logger, "La posicion del ultimo puntero del bloque de PI es: %d \n", posicion_ultimo_puntero);
        
        // Accedemos al bloque de PI 
        //TODO: logger acceso a bloque aca (aclarar que es un bloque de punteros y no de datos)
        //usleep(config_filesystem.RETARDO_ACCESO_BLOQUE * 1000); 

        for( ; bloques_a_quitar > 0; bloques_a_quitar--) {
        
            uint32_t puntero = bloque_indirecto[posicion_ultimo_puntero / sizeof(uint32_t)];
            
            log_debug(logger, "Obtuve el puntero (bloque) %u \n", puntero);

            log_warning(logger, "Acceso a Bitmap - Bloque: %d, Estado: %d \n", puntero, bitarray_test_bit(bitarray_bloques, puntero)); //LOG ACCESO A BITMAP
            bitarray_clean_bit(bitarray_bloques, puntero);
            sincronizar_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);

            // Pongo la entrada del bloque de punteros en cero para evitar que la funcion "agrandar_archivo" se lo saltee
            bloque_indirecto[posicion_ultimo_puntero / sizeof(uint32_t)] = 0;

            posicion_ultimo_puntero -= sizeof(uint32_t);
            
        }


    }

    // Cambiamos el tamanio del archivo en la entrada de FCB y en memoria
    
    archivo->tamanio = tamanio_nuevo;

    config_set_value(fcb_archivo, "TAMANIO_ARCHIVO", string_itoa(archivo->tamanio));

    config_save_in_file(fcb_archivo, file_path);
    
    config_destroy(fcb_archivo);
}



int calcular_posicion_ultimo_puntero(t_fcb* archivo) {

    int posicion_bloque_punteros = archivo->puntero_indirecto * info_superbloque.BLOCK_SIZE;
    
    log_debug(logger, "Posicion bloque ptrs: %d", posicion_bloque_punteros);
    
    int offset = 0;

    uint32_t* bloque_indirecto = (uint32_t*)archivo_bloques_mapeado;

    while(bloque_indirecto[(posicion_bloque_punteros + offset) / sizeof(uint32_t)] != 0) {
        offset += sizeof(uint32_t);
        log_debug(logger, "Offset: %d", offset);
    }

    return (posicion_bloque_punteros + offset - sizeof(uint32_t));
}


void agrandar_archivo(t_fcb* archivo, int tamanio_nuevo) {
    
    char file_path[4096];

    // Concatena en una variable e imprime dicha variable
    snprintf(file_path, 4096, "fcb/%s.dat", archivo->nombre);

    t_config* fcb_archivo = config_create(file_path);

    //FALTA AGREGAR PARA QUE SE MODIFIQUE EL ARCHIVO DE FCB CON LA CLAVE - VALOR -> Por no agregar esto estaba fallando

    uint32_t bloque_libre;

    // Obtenemos la cantidad de bloques que tiene el archivo actualmente
    int cant_bloques_actual = ceil( (float) (archivo->tamanio) / info_superbloque.BLOCK_SIZE);

    // Obtenemos la cantidad de bloques de datos que va a tener el archivo truncado
    int cant_bloques_nuevos = ceil( (float) tamanio_nuevo / info_superbloque.BLOCK_SIZE);

    int bloques_de_datos_por_asignar = cant_bloques_nuevos - cant_bloques_actual;

    // Tiene 1 PD y tendremos que asignarle 1 PI y N PD del PI

    // no tiene el PD asignado
    if(archivo->puntero_directo == -1) {
        bloque_libre = buscar_proximo_bloque_libre();
        archivo->puntero_directo = bloque_libre;
        char* string_aux = string_itoa(archivo->puntero_directo);

        config_set_value(fcb_archivo, "PUNTERO_DIRECTO", string_aux);
        config_save_in_file(fcb_archivo, file_path);

        free(string_aux);
        
        log_warning(logger, "Acceso a Bitmap - Bloque: %d, Estado: %d \n", bloque_libre, bitarray_test_bit(bitarray_bloques, bloque_libre)); //LOG ACCESO A BITMAP 
        bitarray_set_bit(bitarray_bloques, bloque_libre);
        sincronizar_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);

        bloques_de_datos_por_asignar--;
    } 


    // En este caso, el archivo no tiene un bloque de punteros asignado (PI)
    
    /*if(archivo->tamanio <= info_superbloque.BLOCK_SIZE && tamanio_nuevo > info_superbloque.BLOCK_SIZE)*/
    if(archivo->puntero_indirecto == -1 && cant_bloques_nuevos > 1) {    
        
        // busco el prox bloque libre y se lo asigno al bloque de punteros
        bloque_libre = buscar_proximo_bloque_libre();
        log_debug(logger, "El bloque libre que encontre para el PUNTERO INDIRECTO es: %d \n", bloque_libre);
        archivo->puntero_indirecto = bloque_libre;

        char* string_aux = string_itoa(archivo->puntero_indirecto);
        
        config_set_value(fcb_archivo, "PUNTERO_INDIRECTO", string_aux);

        free(string_aux);

        config_save_in_file(fcb_archivo, file_path);
        
        log_warning(logger, "Acceso a Bitmap - Bloque: %d, Estado: %d \n", bloque_libre, bitarray_test_bit(bitarray_bloques, bloque_libre)); //LOG ACCESO A BITMAP 
        bitarray_set_bit(bitarray_bloques, bloque_libre);      // i es el numero de bloque libre en el bitmap
        sincronizar_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);
    }

    // El caso donde archivo->tamanio > tamanio_bloque, ya tenemos asignado el PI
    // Solo queda asignar los punteros del bloque de punteros
    size_t offset = 0;
    int posicion_bloque_punteros = archivo->puntero_indirecto * info_superbloque.BLOCK_SIZE;
    
    // Al for solo tenemos que entrar si tenemos PI
    for( ; bloques_de_datos_por_asignar > 0; bloques_de_datos_por_asignar--) {
        
        bloque_libre = buscar_proximo_bloque_libre();

        log_warning(logger, "Acceso a Bitmap - Bloque: %d, Estado: %d \n", bloque_libre, bitarray_test_bit(bitarray_bloques, bloque_libre)); //LOG ACCESO A BITMAP 
        bitarray_set_bit(bitarray_bloques, bloque_libre);     
        sincronizar_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);

        log_debug(logger, "Escribiendo puntero %d en el bloque de puntero indirecto %u en la posicion %d \n", bloque_libre, archivo->puntero_indirecto, posicion_bloque_punteros);
        usleep(config_filesystem.RETARDO_ACCESO_BLOQUE * 1000); // Retardo acceso a bloque

        uint32_t* bloque_indirecto = (uint32_t*)archivo_bloques_mapeado;
        while(bloque_indirecto[(posicion_bloque_punteros + offset) / sizeof(uint32_t)] != 0) {
            offset += sizeof(uint32_t);
        }

        // Voy al bloque de PI y despues escribo y nos vamos moviendo dentro de ese mismo bloque
        memcpy(archivo_bloques_mapeado + posicion_bloque_punteros + offset, &bloque_libre, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        sincronizar_archivo(archivo_bloques_mapeado, tamanio_archivo_bloques);
    }

    // Cambiamos el tamanio del archivo en la entrada de FCB y en memoria

    archivo->tamanio = tamanio_nuevo;

    char* string_aux = string_itoa(archivo->tamanio);

    config_set_value(fcb_archivo, "TAMANIO_ARCHIVO", string_aux);

    config_save_in_file(fcb_archivo, file_path);
    
    free(string_aux);
    config_destroy(fcb_archivo);
}

// Busca un bloque libre en el bitarray
uint32_t buscar_proximo_bloque_libre() {

    for(int i = 0; i < bitarray_get_max_bit(bitarray_bloques); i++) {
        if(bitarray_test_bit(bitarray_bloques, i) == 0)
        {
            log_debug(logger, "El proximo bloque libre es el bloque %d \n", i);
            return i;
        }
    }
    return -1;    
}


// Busca el proximo bloque del archivo correspondiente al puntero que se paso
uint32_t buscar_bloque(t_fcb* archivo, int puntero, bool* acceso_bloque_punteros, char* nombre_archivo) {

    // Convertimos a vector el archivo de bloques para accederlo y buscar los PD del PI
    uint32_t* archivo_bloques = (uint32_t*)archivo_bloques_mapeado;
    
    // Tenemos que averiguar a que BLOQUE DEL ARCHIVO corresponde el puntero que nos mandaron
    int bloque_archivo = floor((float)puntero / info_superbloque.BLOCK_SIZE);

    // Ahora tenemos que buscar a que BLOQUE DEL FS corresponde ese bloque de archivo
    uint32_t bloque_filesystem;
    int posicion_bloque_punteros = (archivo->puntero_indirecto * info_superbloque.BLOCK_SIZE);
    int offset = 0;

    
    // Si el bloque del archivo es el bloque 0, entonces ya sabemos que es el bloque que esta en el puntero directo
    if(bloque_archivo == 0) {
        bloque_filesystem = archivo->puntero_directo;
    }
    // En cambio, si es mayor a 0, tenemos que buscarlo en el bloque de puntero indirecto
    else if(bloque_archivo > 0) {
        
        if(!(*acceso_bloque_punteros)) {
            log_debug(logger, "Tengo que acceder al bloque de puntero indirecto \n");
            log_warning(logger, "Acceso Bloque Punteros - Archivo: %s - Bloque File System: %d \n", nombre_archivo, archivo->puntero_indirecto); //LOG ACCESO A BLOQUE
            usleep(config_filesystem.RETARDO_ACCESO_BLOQUE * 1000);
            *acceso_bloque_punteros = 1;
        }


        // Si el bloque es 1 sabemos que es la posicion 0, si es bloque 2 es la posicion 1 y asi...
        // Bloque archivo 1 -> Posicion 0 desde el principio del bloque de PI
        // Bloque archivo 2 -> Posicion 1 desde el principio del bloque de PI (o sea segundo puntero escrito en PI)
        // Bloque archivo N -> Posicion N+1 desde el principio del bloque de PI

        // Determinamos el offset
        offset = (bloque_archivo - 1) * sizeof(uint32_t);

        bloque_filesystem = archivo_bloques[(posicion_bloque_punteros + offset) / sizeof(uint32_t)];                    
    }

    log_debug(logger, "Tengo que acceder al bloque del archivo %d \n", bloque_archivo);

    log_debug(logger, "Tengo que acceder al bloque del FileSystem %d \n", bloque_filesystem);

    log_warning(logger, "Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System: %d \n", nombre_archivo, bloque_archivo, bloque_filesystem); //LOG ACCESO A BLOQUE

    return bloque_filesystem;
}

void escribir_bloque(uint32_t bloque_a_escribir, char* valor_a_escribir, int cantidad_bytes, int offset) {

    int posicion_bloque_a_escribir = bloque_a_escribir * info_superbloque.BLOCK_SIZE;
    
    // Escribimos esos datos en disco
    memcpy(archivo_bloques_mapeado + posicion_bloque_a_escribir, valor_a_escribir + offset, cantidad_bytes);
    sincronizar_archivo(archivo_bloques_mapeado, tamanio_archivo_bloques);  
}

void leer_bloque(uint32_t bloque_a_leer, char* informacion_leida, int cantidad_bytes) {

    int posicion_bloque_a_leer = bloque_a_leer * info_superbloque.BLOCK_SIZE;
    
    // Leemos esos datos de disco
    memcpy(informacion_leida, archivo_bloques_mapeado + posicion_bloque_a_leer, cantidad_bytes);
}
