#include "procesar_kernel_cpu.h"

// ------------------------------ PROCESAMIENTO DEL KERNEL EN CPU ------------------------------ //

void procesar_kernel_cpu() {
    while(1) {
        // Aca pensaba que había que usar semáforos pero no, el recv se encarga de recibir solo cuando el otro hace un send, sino se queda clavado.
        op_code cod_op = recibir_operacion(cliente_kernel);
        
        switch((int)cod_op) {
            case CONTEXTO_EJECUCION:
            {
                t_contexto_ejecucion* contexto = crear_contexto();

                
                if(!recv_contexto(cliente_kernel, contexto)) {
                    log_error(logger, "Fallo recibiendo CONTEXTO");
                    break;
                }

                ejecutar_proceso(contexto); // Se encarga también del desalojo del proceso, no hace falta poner nada abajo de esto

                liberar_contexto(contexto);

                break;
            }
            case -1:
            {
			    log_error(logger, "El cliente se desconecto. Terminando Servidor \n");
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