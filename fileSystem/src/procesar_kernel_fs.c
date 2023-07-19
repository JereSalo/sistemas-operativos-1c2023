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
                t_fcb* archivo = buscar_archivo_en_lista_fcbs(nombre_archivo);
                

                //obtenemos el puntero directo y ahi ya sabemos en que espacio del archivo mapeado en memoria vamos a tener que leer
                 


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
                log_debug(logger, "Recibi solicitud de truncar archivo por parte de Kernel \n");
                
                char nombre_archivo[64];
                int tamanio;
                int pid;

                recv_string(cliente_kernel, nombre_archivo);
                RECV_INT(cliente_kernel, tamanio);
                RECV_INT(cliente_kernel, pid);      // Para despues mandarle al Kernel que proceso se debe desbloquear 

                // Sacamos cuantos bloques vamos a agregar o eliminar
                int cant_bloques_nuevos = ceil(tamanio / info_superbloque.BLOCK_SIZE);                  
                
                t_fcb* archivo = buscar_archivo_en_lista_fcbs(nombre_archivo);

                if(archivo->tamanio > tamanio) {
                    //achicar_archivo();
                }
                else if(archivo->tamanio < tamanio) {
                    
                    agrandar_archivo(archivo, tamanio, cant_bloques_nuevos);
                }
                
                
                mostrar_bitarray();
                //mostrar_contenido_archivo("bloques.dat");


                uint32_t* data_as_chars = (uint32_t*)archivo_bloques_mapeado;
                for (int i = 0; i < tamanio_archivo_bloques; i++) {
                    printf("IMPRIMO VALOR DEL ARCHIVO %u \n", data_as_chars[i]);
                }


                //read_file_bytes("bloques.dat");
                // Cambiamos el tamanio del FCB en memoria
                // Cambiamos el tamanio del FCB en el archivo de directorio
                




                break;
            }
        }
    }
}



void agrandar_archivo(t_fcb* archivo, int tamanio, int cant_bloques_nuevos) {
    
    // Vamos a asignarle al archivo un PD y un PI con X cantidad de PD
    if(tamanio > info_superbloque.BLOCK_SIZE) {
        cant_bloques_nuevos++; // Le sumamos el puntero indirecto.
    }
    
    size_t offset;
    int posicion_bloque;

    for(int numero_bloque = 0; numero_bloque < cant_bloques_nuevos; numero_bloque++){
        //numero_bloque es nuestro iterador (i)
        
        uint32_t bloque_libre = buscar_proximo_bloque_libre();
        int bloque_puntero_indirecto;
        
        // Seteamos ese bloque como ocupado
        bitarray_set_bit(bitarray_bloques, numero_bloque);      // i es el numero de bloque libre en el bitmap
        sincronizar_archivo(archivo_bitmap_mapeado, tamanio_archivo_bitmap);
        
        // Si el numero de bloque es 1, entonces ya sabemos que ese va a ser el bloque de PI
        if(numero_bloque == 1) {
            
            log_debug(logger, "El archivo %s ocupara un puntero indirecto \n", archivo->nombre);

            //memcpy(archivo_bloques_mapeado + posicion_bloque + offset, &bloque_libre, sizeof(uint32_t));

            bloque_puntero_indirecto = bloque_libre;    // me guardo el numero de bloque correspondiente al PI
        
            // Ahora buscamos a que posicion del archivo de bloques corresponde el bloque de PI
            posicion_bloque = bloque_puntero_indirecto * info_superbloque.BLOCK_SIZE;
            offset = 0;
        }
        
        // Si el numero de bloque es mayor a 1, entonces sabemos que esos bloques son PD del PI y los escribimos ahi
        if(numero_bloque > 1) {
            
            log_debug(logger, "Escribiendo puntero %d en el bloque de puntero indirecto \n", bloque_libre);

            // Voy al bloque de PI y despues escribo y nos vamos moviendo dentro de ese mismo bloque
            memcpy(archivo_bloques_mapeado + posicion_bloque + offset, &bloque_libre, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            sincronizar_archivo(archivo_bloques_mapeado, tamanio_archivo_bloques);
        }
    }
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