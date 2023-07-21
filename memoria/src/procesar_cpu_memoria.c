#include "procesar_cpu_memoria.h"

void procesar_cpu_memoria() {
    while(1) {
        op_code cod_op = recibir_operacion(cliente_cpu);
        
        // Avisamos que el proceso deja de correr, y que puede ingresar otro en la cpu
        
        switch((int)cod_op) {
            case SOLICITUD_LECTURA:
            {
                int direccion_fisica;
                int longitud;
                RECV_INT(cliente_cpu, direccion_fisica);
                RECV_INT(cliente_cpu, longitud);

                char* datos_leidos = malloc(longitud + 1);

                usleep(config_memoria.RETARDO_MEMORIA * 1000); // Acceso a espacio de usuario
                memcpy(datos_leidos, memoria_principal + direccion_fisica, longitud);
                
                datos_leidos[longitud] = '\0';

                printf("DATOS LEIDOS DE MEMORIA: %s", datos_leidos);
                
                send_string(cliente_cpu, datos_leidos);

                free(datos_leidos);

                break;
            }

            case SOLICITUD_ESCRITURA:
            {
                int direccion_fisica;
                int longitud;
                RECV_INT(cliente_cpu, direccion_fisica);
                RECV_INT(cliente_cpu, longitud);

                char* valor_a_escribir = (char*)(recv_paquete(cliente_cpu, (size_t)longitud));

                usleep(config_memoria.RETARDO_MEMORIA * 1000); // Acceso a espacio de usuario
                memcpy(memoria_principal + direccion_fisica, valor_a_escribir, longitud);

                
                //printf("DATOS ESCRITOS EN MEMORIA: %s", valor_a_escribir);
                
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
    }
}