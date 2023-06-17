#include "procesar_kernel_memoria.h"

void procesar_kernel_memoria() {
    while(1) {
        op_code cod_op = recibir_operacion(cliente_kernel);
        
        // Avisamos que el proceso deja de correr, y que puede ingresar otro en la cpu
        
        switch((int)cod_op) {
            case SOLICITUD_TABLA:
            {
                log_info(logger,"RECIBI UNA SOLICITUD DE TABLA DE SEGMENTOS \n");
            
                t_list* tabla_segmentos = list_create();
                
                // Metemos el segmento 0 en la tabla de segmentos inicial
                list_add(tabla_segmentos, segmento_cero);


                log_info(logger,"SEGMENTO CERO CARGADO EN TABLA \n");

                // Mandamos la tabla de segmentos 
                send_tabla_segmentos(cliente_kernel, tabla_segmentos);

                break;
            }
            case -1:
            {
			    log_error(logger, "El cliente KERNEL se desconecto. Terminando Servidor \n");
			    return;
            }
		    default:
            {
			    log_error(logger,"Operación desconocida. Hubo un problemita! \n");
			    break;
            }
        }
    }
}