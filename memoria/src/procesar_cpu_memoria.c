#include "procesar_cpu_memoria.h"

void procesar_cpu_memoria() {
    while(1) {
        op_code cod_op = recibir_operacion(cliente_cpu);
        
        // Avisamos que el proceso deja de correr, y que puede ingresar otro en la cpu
        pthread_mutex_lock(&mutex_falopa);
        
        switch((int)cod_op) {
            case SOLICITUD_LECTURA:
            {
                int direccion_fisica;
                int longitud;
                int pid;
                RECV_INT(cliente_cpu, direccion_fisica);
                RECV_INT(cliente_cpu, longitud);
                RECV_INT(cliente_cpu, pid);             //CHEQUEAR


                char* datos_leidos = malloc(longitud + 1);


                log_warning(logger, "PID: %d - Accion: LEER - Direccion fisica: %d - Tamanio: %d - Origen CPU \n", pid, direccion_fisica, longitud); //LOG ACCESO A ESPACIO DE USUARIO
                usleep(config_memoria.RETARDO_MEMORIA * 1000); // Acceso a espacio de usuario
                memcpy(datos_leidos, memoria_principal + direccion_fisica, longitud);
                
                datos_leidos[longitud] = '\0';

                log_info(logger, "Datos leidos de memoria por peticion de CPU: %s \n", datos_leidos);
                
                send_string(cliente_cpu, datos_leidos);

                free(datos_leidos);

                break;
            }

            case SOLICITUD_ESCRITURA:
            {
                int direccion_fisica;
                int longitud;
                int pid;
                RECV_INT(cliente_cpu, direccion_fisica);
                RECV_INT(cliente_cpu, longitud);
                
                char* valor_a_escribir = (char*)(recv_paquete(cliente_cpu, (size_t)longitud));
                
                RECV_INT(cliente_cpu, pid);             //CHEQUEAR

                log_warning(logger, "PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamanio: %d - Origen CPU \n", pid, direccion_fisica, longitud); //LOG ACCESO A ESPACIO DE USUARIO
                usleep(config_memoria.RETARDO_MEMORIA * 1000); // Acceso a espacio de usuario
                memcpy(memoria_principal + direccion_fisica, valor_a_escribir, longitud);

                
                log_info(logger, "Datos escritos en memoria por peticion de CPU: %s \n", valor_a_escribir);
                
                free(valor_a_escribir);

                //Debug
                //leer_memoria();

                send_string(cliente_cpu, "OK");

                break;
            }
            case -1:
            {
                log_error(logger, "El cliente CPU se desconecto. Terminando Servidor \n");
			    return;
            }
            default:
            {
                log_error(logger, "No entiendo el mensaje de CPU");
            }
            
        }
        pthread_mutex_unlock(&mutex_falopa);
    }
}