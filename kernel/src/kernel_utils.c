#include "kernel_utils.h"

t_log* logger;
int pid_counter = 1;

t_pcb* inicializar_pcb(int cliente_socket) {
    // Recibimos las instrucciones
    t_list* instrucciones_recibidas = list_create();

    if(!recv_instrucciones(cliente_socket, instrucciones_recibidas)){
        log_error(logger, "Fallo recibiendo INSTRUCCIONES");
    }

    // Creamos el PCB

    // semaforo wait

    t_pcb* pcb = crear_pcb(pid_counter, instrucciones_recibidas);
    pid_counter++;

    // semaforo post
   
    //list_destroy_and_destroy_elements(instrucciones_recibidas,free);           ESTO LO VAMOS A HACER EN OTRO LADO

    return pcb;
}

t_pcb* crear_pcb(int pid, t_list* lista_instrucciones) {

    t_pcb* pcb = malloc(sizeof(t_pcb));

    pcb->pid = pid;
    pcb->instrucciones = lista_instrucciones;
    pcb->estado = NEW;
    //pcb->registros_cpu;                       TODO: ver como inicializar los registros
    pcb->tabla_segmentos = list_create();       //TODO: la dejamos como vacia pero la tabla la va a armar la memoria
    pcb->estimacion_prox_rafaga = 0;            //TODO: llega de kernel.config la estimacion inicial
    pcb->tiempo_llegada_ready = 0;
    pcb->tabla_archivos_abiertos = list_create();


    return pcb;
}

void procesar_conexion_kernel(void* void_cliente_socket) {
    int cliente_socket = (intptr_t) void_cliente_socket;
    while(1) {
        // Aca pensaba que había que usar semáforos pero no, el recv se encarga de recibir solo cuando el otro hace un send, sino se queda clavado.
        op_code cod_op = recibir_operacion(cliente_socket);
        t_pcb* pcb;

        switch((int)cod_op) {
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

                pcb = inicializar_pcb(cliente_socket);

                mostrar_lista(pcb->instrucciones);

                break;
            }
            case -1:
            {
			    log_info(logger, "El cliente se desconecto. Terminando Servidor");
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