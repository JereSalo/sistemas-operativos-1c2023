#include "cpu_utils.h"

void procesar_conexion_cpu(int cliente_socket) {
    while(1) {
        // Aca pensaba que había que usar semáforos pero no, el recv se encarga de recibir solo cuando el otro hace un send, sino se queda clavado.
        op_code cod_op = recibir_operacion(cliente_socket);
        
        switch(cod_op) {
            case NUMERO: // Este está de prueba todavía
            {
                printf("El cop que me llegó es Número\n");
                int numero_recibido;

                if(!recv_numero(cliente_socket, &numero_recibido)) {
                    log_error(logger, "Fallo recibiendo NUMERO");
                    break;
                }

                log_info(logger, "RECIBI EL MENSAJE %d", numero_recibido);

                break;
            }
            case INSTRUCCIONES:
            {
                printf("El cop que me llegó es Instrucciones\n");
                t_list* instrucciones_recibidas = list_create();

                if(!recv_instrucciones(cliente_socket,instrucciones_recibidas)){
                    log_error(logger, "Fallo recibiendo INSTRUCCIONES");
                    break;
                }

                log_info(logger, "RECIBI LAS INSTRUCCIONES\n");

                mostrar_lista(instrucciones_recibidas);

                // ACA VIENE TODO EL COMPORTAMIENTO DE LA INSTRUCCION

                list_destroy_and_destroy_elements(instrucciones_recibidas,free);
                break;
            }
            case -1:
            {
			    log_error(logger, "El cliente se desconecto. Terminando Servidor");
			    return;
            }
		    default:
            {
			    log_warning(logger,"Operación desconocida. Hubo un problemita !");
			    break;
            }
        }
    }
}