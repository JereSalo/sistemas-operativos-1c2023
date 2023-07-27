#include "procesar_filesystem_memoria.h"

void procesar_filesystem_memoria(){
    while(1) {
        op_code cod_op = recibir_operacion(cliente_filesystem);

        // Avisamos que el proceso deja de correr, y que puede ingresar otro en la cpu
        pthread_mutex_lock(&mutex_falopa);

        switch((int)cod_op){
            case SOLICITUD_ESCRITURA:
            {
                // Recibe de FS datos de disco y los pone en memoria para que despues se puedan consultar

                int direccion_fisica;
                int longitud;
                int pid;
                RECV_INT(cliente_filesystem, direccion_fisica);
                RECV_INT(cliente_filesystem, longitud);


                char* valor_a_escribir = (char*)(recv_paquete(cliente_filesystem, (size_t)longitud));

                RECV_INT(cliente_filesystem, pid);             //CHEQUEAR


                log_warning(logger, "PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamanio: %d - Origen FILESYSTEM \n", pid, direccion_fisica, longitud); //LOG ACCESO A ESPACIO DE USUARIO
                usleep(config_memoria.RETARDO_MEMORIA * 1000); // Acceso a espacio de usuario
                memcpy(memoria_principal + direccion_fisica, valor_a_escribir, longitud);
                

                log_info(logger, "Datos escritos en memoria por peticion FS: %s \n", valor_a_escribir);
                
                free(valor_a_escribir);

                //Debug
                //leer_memoria();

                send_string(cliente_filesystem, "OK");

                break;
            }
            case SOLICITUD_LECTURA:
            {
                // Leemos de memoria para que despues FS escriba en disco
                
                int direccion_fisica;
                int longitud;
                int pid;
                RECV_INT(cliente_filesystem, direccion_fisica);
                RECV_INT(cliente_filesystem, longitud);
                RECV_INT(cliente_filesystem, pid);             //CHEQUEAR

                char* datos_leidos = malloc(longitud + 1);

                log_warning(logger, "PID: %d - Accion: LEER - Direccion fisica: %d - Tamanio: %d - Origen FILESYSTEM \n", pid, direccion_fisica, longitud); //LOG ACCESO A ESPACIO DE USUARIO
                usleep(config_memoria.RETARDO_MEMORIA * 1000); // Acceso a espacio de usuario
                memcpy(datos_leidos, memoria_principal + direccion_fisica, longitud);
                
                datos_leidos[longitud] = '\0';

                log_info(logger, "Datos leidos de memoria por peticion de FS: %s \n", datos_leidos);
                
                send_string(cliente_filesystem, datos_leidos);

                free(datos_leidos);

                break;
            }
        }
        pthread_mutex_unlock(&mutex_falopa);
    }
}