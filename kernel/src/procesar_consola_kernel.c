#include "procesar_consola_kernel.h"

// ------------------------------ PROCESAMIENTO DE CONSOLA EN KERNEL ------------------------------ //

void procesar_consola(void* void_cliente_consola) {
    
    int cliente_consola = (intptr_t) void_cliente_consola;
    
    while(1) {
        op_code cod_op = recibir_operacion(cliente_consola);
        t_pcb* pcb;

        switch((int)cod_op) {
            case INSTRUCCIONES:
            {
                // log_info(logger, "Me llego el codigo de operacion INSTRUCCIONES \n");

                // Inicializamos el PCB de un proceso (esto implica crearlo)
                pcb = inicializar_pcb(cliente_consola);
                
                // Agregamos el proceso creado a NEW
                pthread_mutex_lock(&mutex_new);
                queue_push(procesos_en_new, pcb);
                pthread_mutex_unlock(&mutex_new);

                log_warning(logger, "Se crea el proceso %d en NEW \n", pcb->pid); //LOG CREACION DE PROCESO

                // Avisamos que agregamos un nuevo proceso a NEW
                sem_post(&cant_procesos_new);   

                // Enviar confirmacion de recepcion a consola
                SEND_INT(cliente_consola, 1);
                
                break;
            }
            case -1:
            {
			    log_error(logger, "El cliente CONSOLA se desconecto. Terminando Servidor \n");
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