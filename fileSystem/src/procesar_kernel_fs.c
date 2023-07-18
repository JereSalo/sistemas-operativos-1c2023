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
                RECV_INT(cliente_kernel, cantidad_bytes);
                RECV_INT(cliente_kernel, direccion_fisica);
                RECV_INT(cliente_kernel, pid);

                //leemos a partir del puntero una cierta cantidad de bytes
                
                
                //buscamos al archivo en la lista de FCBs -> tiene que estar si o si
                t_fcb* archivo = buscar_archivo_en_lista_fcbs(nombre_archivo);
                

                //obtenemos el puntero directo y ahi ya sabemos en que espacio del archivo mapeado en memoria vamos a tener que leer
                archivo->puntero_directo


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

                archivo->nombre = nombre_archivo;
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

                log_warning(logger, "Abrir archivo: %s \n", nombre_archivo); 

                if(archivo == NULL) {
                    SEND_INT(cliente_kernel, 1);
                }
                else {
                    SEND_INT(cliente_kernel, 0);
                }

                log_debug(logger, "Envie respuesta a Kernel"); // LOG ABRIR ARCHIVO

                free(archivo);
                break;
            }
            case SOLICITUD_TRUNCAR_ARCHIVO:
                break;
        }
    }
}