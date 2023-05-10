#include "kernel_utils.h"

t_log* logger;
int pid_counter = 1;

void inicializar_semaforos(grado_de_multiprogramacion) {
    
    pthread_mutex_init(&mutex_new, NULL);
    pthread_mutex_init(&mutex_ready, NULL);
    
    sem_init(&cant_procesos_new, 0, 0);
    sem_init(&cant_procesos_ready, 0, 0);

    //uint grado_de_multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
    sem_init(&maximo_grado_de_multiprogramacion, 0, grado_de_multiprogramacion);
}

void inicializar_colas() {
    procesos_en_new = queue_create();
    procesos_en_ready = list_create();
}

t_pcb* inicializar_pcb(int cliente_socket) {
    // Recibimos las instrucciones
    t_list* instrucciones_recibidas = list_create();

    if(!recv_instrucciones(cliente_socket, instrucciones_recibidas)){
        log_error(logger, "Fallo recibiendo INSTRUCCIONES");
    }

    // Creamos el PCB

    t_pcb* pcb = crear_pcb(pid_counter, instrucciones_recibidas);
    pid_counter++;
   
    //list_destroy_and_destroy_elements(instrucciones_recibidas,free);           ESTO LO VAMOS A HACER EN OTRO LADO

    return pcb;
}

t_pcb* crear_pcb(int pid, t_list* lista_instrucciones) {

    t_pcb* pcb = malloc(sizeof(t_pcb));

    pcb->pid = pid;
    pcb->instrucciones = lista_instrucciones;
    //pcb->estado = NEW;
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
                
                //mostrar_lista(pcb->instrucciones);
                
                //LO TENEMOS QUE MANDAR A LA COLA DE NEW

                // una vez que lo agregamos a new, hacemos un signal al semaforo cant_procesos_new;

                pthread_mutex_lock(&mutex_new);
                queue_push(procesos_en_new, pcb);
                pthread_mutex_unlock(&mutex_new);


                log_info(logger, "Agregue un proceso a la cola de NEW");

                sem_post(&cant_procesos_new);   
                
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