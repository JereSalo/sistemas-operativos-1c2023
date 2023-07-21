#include "procesar_kernel_fs.h"

void procesar_kernel_filesystem(){
    while(1) {
        op_code cod_op = recibir_operacion(cliente_kernel);

        switch((int)cod_op) {
            case SOLICITUD_LECTURA_DISCO:
            {
                char nombre_archivo[64];
                recv_string(cliente_kernel, nombre_archivo);

                int cantidad_bytes;
                int direccion_fisica;
                int pid;
                int puntero;
                RECV_INT(cliente_kernel, cantidad_bytes);
                RECV_INT(cliente_kernel, direccion_fisica);
                RECV_INT(cliente_kernel, pid);
                RECV_INT(cliente_kernel, puntero);

                //leemos a partir del puntero una cierta cantidad de bytes
                
                
                //buscamos al archivo en la lista de FCBs -> tiene que estar si o si
                //t_fcb* archivo = buscar_archivo_en_lista_fcbs(nombre_archivo); ¡¡¡ MEPA QUE ESTO NO VA !!!
                
                char* informacion_leida = malloc(1088 + 1); // buffer con tam max archivo (TODO)
                //obtenemos el puntero directo y ahi ya sabemos en que espacio del archivo mapeado en memoria vamos a tener que leer

                memcpy(informacion_leida, archivo_bloques_mapeado + puntero, cantidad_bytes);
                
                
                


                //leer_archivo();

                //despues de leer esos datos los mandamos a memoria para que los escriba en su espacio

                //mandamos a memoria
  
                //send_termine_loco(kernel);

                break;
            }
            case SOLICITUD_ESCRITURA_DISCO:
                break;
            case SOLICITUD_CREAR_ARCHIVO:
            {
                log_debug(logger, "Recibi solicitud de crear archivo por parte de Kernel");
                
                t_fcb* archivo = malloc(sizeof(t_fcb));
                char nombre_archivo[64];
                recv_string(cliente_kernel, nombre_archivo);

                archivo->nombre = strdup(nombre_archivo);
                archivo->tamanio = 0;
                archivo->puntero_directo = -1;
                archivo->puntero_indirecto = -1;

                list_add(lista_fcbs, archivo);


                log_debug(logger, "Voy a crear la entrada de directorio");
                crear_entrada_directorio(nombre_archivo);

                
                log_warning(logger, "Crear archivo: %s \n", nombre_archivo); //LOG CREAR ARCHIVO
                
                break;
            }
            case SOLICITUD_ABRIR_ARCHIVO:
            {
                log_debug(logger, "Recibi solicitud de abrir archivo por parte de Kernel");
                
                char nombre_archivo[64];
                recv_string(cliente_kernel, nombre_archivo);


                // Verifica si existe el archivo -> buscamos en la lista de fcbs
                t_fcb* archivo = buscar_archivo_en_lista_fcbs(nombre_archivo);

                //log_debug(logger, "Archivo encontrado: %s", archivo->nombre);
                
                log_warning(logger, "Abrir archivo: %s \n", nombre_archivo); 

                if(archivo == NULL) {
                    log_debug(logger, "El archivo no existe \n"); 
                    SEND_INT(cliente_kernel, 1);
                }
                else {
                    log_debug(logger, "El archivo existe, no es necesario crearlo \n");
                    SEND_INT(cliente_kernel, 0);
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
                log_debug(logger, "Recibi solicitud de truncar archivo por parte de Kernel: %d \n", tamanio);

                // Sacamos cuantos bloques vamos a agregar o eliminar
                int cant_bloques_nuevos = ceil(tamanio / info_superbloque.BLOCK_SIZE);                  
                
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
                
                //archivo->tamanio = tamanio;
                
                mostrar_bitarray();
                //mostrar_contenido_archivo("bloques.dat");

                log_debug(logger, "Mostrando contenido archivo de bloques: \n");
                uint32_t* data_as_chars = (uint32_t*)archivo_bloques_mapeado;
                for (int i = 0; i < 20; i++) {
                    int nro_bloque = ceil(i/4);
                    //if(data_as_chars[i] != 0)
                    printf("VALOR ESCRITO EN BLOQUE NRO %d - %u \n", nro_bloque, data_as_chars[i]);
                    
                }


                //read_file_bytes("bloques.dat");
                // Cambiamos el tamanio del FCB en memoria
                // Cambiamos el tamanio del FCB en el archivo de directorio
                

                // mandamos el PID al Kernel para que desbloquee al proceso
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

        // Sacamos al puntero directo del bitmap
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
        

        for( ; bloques_a_quitar > 0; bloques_a_quitar--) {
        
            uint32_t puntero = bloque_indirecto[posicion_ultimo_puntero / sizeof(uint32_t)];
            
            log_debug(logger, "Obtuve el puntero (bloque) %u \n", puntero);

            bitarray_clean_bit(bitarray_bloques, puntero);
            sincronizar_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);

            // Pongo la entrada del bloque de punteros en cero para evitar que la funcion "agrandar_archivo" se lo saltee
            bloque_indirecto[posicion_ultimo_puntero / sizeof(uint32_t)] = 0;

            posicion_ultimo_puntero -= sizeof(uint32_t);
            
        }


    }

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
    int cant_bloques_nuevos = ceil( (float)tamanio_nuevo / info_superbloque.BLOCK_SIZE);

    int bloques_de_datos_por_asignar = cant_bloques_nuevos - cant_bloques_actual;

    // Tiene 1 PD y tendremos que asignarle 1 PI y N PD del PI

    log_debug(logger, "Valor de bloques por asignar antes del 1er IF: %d \n", bloques_de_datos_por_asignar);

    // no tiene el PD asignado
    if(archivo->puntero_directo == -1) {
        bloque_libre = buscar_proximo_bloque_libre();
        archivo->puntero_directo = bloque_libre;

        config_set_value(fcb_archivo, "PUNTERO_DIRECTO", string_itoa(archivo->puntero_directo));
        config_save_in_file(fcb_archivo, file_path);
        
        bitarray_set_bit(bitarray_bloques, bloque_libre);
        sincronizar_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);

        bloques_de_datos_por_asignar--;
    } 

    log_debug(logger, "Valor de bloques por asignar antes del 2do if: %d \n", bloques_de_datos_por_asignar);

    // En este caso, el archivo no tiene un bloque de punteros asignado (PI)
    
    /*if(archivo->tamanio <= info_superbloque.BLOCK_SIZE && tamanio_nuevo > info_superbloque.BLOCK_SIZE)*/
    if(archivo->puntero_indirecto == -1 && cant_bloques_nuevos > 1) {    
        
        // busco el prox bloque libre y se lo asigno al bloque de punteros
        bloque_libre = buscar_proximo_bloque_libre();
        archivo->puntero_indirecto = bloque_libre;
        
        config_set_value(fcb_archivo, "PUNTERO_INDIRECTO", string_itoa(archivo->puntero_indirecto));

        config_save_in_file(fcb_archivo, file_path);
        
        bitarray_set_bit(bitarray_bloques, bloque_libre);      // i es el numero de bloque libre en el bitmap
        sincronizar_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);
    }

    // El caso donde archivo->tamanio > tamanio_bloque, ya tenemos asignado el PI
    // Solo queda asignar los punteros del bloque de punteros
    size_t offset = 0;
    int posicion_bloque_punteros = archivo->puntero_indirecto * info_superbloque.BLOCK_SIZE;
    
    log_debug(logger, "Valor de bloques por asignar antes de entrar al FOR: %d \n", bloques_de_datos_por_asignar);
    // Al for solo tenemos que entrar si tenemos PI
    for( ; bloques_de_datos_por_asignar > 0; bloques_de_datos_por_asignar--) {
        
        bloque_libre = buscar_proximo_bloque_libre();

        bitarray_set_bit(bitarray_bloques, bloque_libre);      // i es el numero de bloque libre en el bitmap
        sincronizar_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);

        log_debug(logger, "Escribiendo puntero %d en el bloque de puntero indirecto %u en la posicion %d \n", bloque_libre, archivo->puntero_indirecto, posicion_bloque_punteros);

        uint32_t* bloque_indirecto = (uint32_t*)archivo_bloques_mapeado;
        while(bloque_indirecto[(posicion_bloque_punteros + offset) / sizeof(uint32_t)] != 0) {
            offset += sizeof(uint32_t);
        }

        // Voy al bloque de PI y despues escribo y nos vamos moviendo dentro de ese mismo bloque
        memcpy(archivo_bloques_mapeado + posicion_bloque_punteros + offset, &bloque_libre, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        sincronizar_archivo(archivo_bloques_mapeado, tamanio_archivo_bloques);
    }



    archivo->tamanio = tamanio_nuevo;

    config_set_value(fcb_archivo, "TAMANIO_ARCHIVO", string_itoa(archivo->tamanio));

    config_save_in_file(fcb_archivo, file_path);
    


    config_destroy(fcb_archivo);
}


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


void read_file_bytes(const char* filename) {
    FILE* file = fopen(filename, "rb"); // Open the file in binary read mode

    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Read each byte from the file
    int byte;
    while ((byte = fgetc(file)) != EOF) {
        if (byte >= 0 && byte <= 255) {
            printf("Byte (char): %c\n", (char)byte);
        } else {
            // If the byte is not within the range of a char, assume it's a uint32_t
            uint32_t value;
            fread(&value, sizeof(uint32_t), 1, file);
            printf("Byte (uint32_t): %u\n", value);
        }
    }

    fclose(file); // Close the file when done
}