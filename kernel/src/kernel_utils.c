#include "kernel_utils.h"

t_log* logger;
t_kernel_config config_kernel;
int pid_counter = 1;

// Colas de los estados de los procesos
t_queue* procesos_en_new;
t_list* procesos_en_ready;
t_pcb* proceso_en_running;

// Semaforos
pthread_mutex_t mutex_new;
pthread_mutex_t mutex_ready;
pthread_mutex_t mutex_running;
sem_t maximo_grado_de_multiprogramacion;
sem_t cant_procesos_new;
sem_t cant_procesos_ready;


void cargar_config_kernel(t_config* config) {
    
    config_kernel.IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    config_kernel.PUERTO_MEMORIA = config_get_int_value(config, "PUERTO_MEMORIA");

    config_kernel.IP_FILESYSTEM = config_get_string_value(config, "IP_FILESYSTEM");
    config_kernel.PUERTO_FILESYSTEM = config_get_int_value(config, "PUERTO_FILESYSTEM");

    config_kernel.IP_CPU = config_get_string_value(config, "IP_CPU");
    config_kernel.PUERTO_CPU = config_get_int_value(config, "PUERTO_CPU");

    config_kernel.PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");

    config_kernel.ALGORITMO_PLANIFICACION = config_get_string_value(config, "IP_ALGORITMO_PLANIFICACION");
    
    
    config_kernel.ESTIMACION_INICIAL = config_get_int_value(config, "ESTIMACION_INICIAL");

    config_kernel.HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");

    config_kernel.GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");

    //FALTAN LAS DOS LISTAS


    log_info(logger, "Config cargada en config_kernel");
}

void inicializar_semaforos() {
    pthread_mutex_init(&mutex_new, NULL);
    pthread_mutex_init(&mutex_ready, NULL);
    pthread_mutex_init(&mutex_running, NULL);
    
    sem_init(&cant_procesos_new, 0, 0);
    sem_init(&cant_procesos_ready, 0, 0);
    sem_init(&maximo_grado_de_multiprogramacion, 0, config_kernel.GRADO_MAX_MULTIPROGRAMACION);
}

void inicializar_colas() {
    procesos_en_new = queue_create();
    procesos_en_ready = list_create();
}

void inicializar_registros(t_registros_cpu* registros) {
    
    for(int i=0; i<4; i++) {
        registros->AX[i] = 0;
        registros->BX[i] = 0;
        registros->CX[i] = 0;
        registros->DX[i] = 0;
    }

    for (int i = 0; i < 8; i++)
    {
        registros->EAX[i] = 0;
        registros->EBX[i] = 0;
        registros->ECX[i] = 0;
        registros->EDX[i] = 0;
    }

    for (int i = 0; i < 16; i++)
    {
        registros->RAX[i] = 0;
        registros->RBX[i] = 0;
        registros->RCX[i] = 0;
        registros->RDX[i] = 0;
    }
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
   
    return pcb;
}

t_pcb* crear_pcb(int pid, t_list* lista_instrucciones) {
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->pid = pid;
    pcb->pc = 0;
    pcb->instrucciones = lista_instrucciones;
    pcb->registros_cpu = malloc(sizeof(t_registros_cpu));
    inicializar_registros(pcb->registros_cpu);                          //TODO: ver como inicializar los registros
    pcb->tabla_segmentos = list_create();                               //TODO: la dejamos como vacia pero la tabla la va a armar la memoria
    pcb->estimacion_prox_rafaga = config_kernel.ESTIMACION_INICIAL;            
    pcb->tiempo_llegada_ready = 0;                                      //TODO: Esto lo tenemos que cambiar por el timestamp
    pcb->tabla_archivos_abiertos = list_create();

    return pcb;
}
/*
void cargar_contexto_de_ejecucion(t_pcb* proceso, t_contexto_ejecucion* contexto_de_ejecucion){
    contexto_de_ejecucion->pid = proceso->pid;
    contexto_de_ejecucion->pc = proceso->pc;

    for(int i=0; i<5; i++){
        contexto_de_ejecucion->registros_cpu.registros_cpu_8[i] = proceso->registros_cpu.registros_cpu_8[i];
    }
    
}
*/


void procesar_conexion_kernel(void* void_cliente_socket) {
    
    int cliente_socket = (intptr_t) void_cliente_socket;
    while(1) {
        op_code cod_op = recibir_operacion(cliente_socket);
        t_pcb* pcb;

        switch((int)cod_op) {
            case INSTRUCCIONES:
            {
                log_info(logger, "Me llego el codigo de operacion INSTRUCCIONES");

                // Inicializamos el PCB de un proceso (esto implica crearlo)
                pcb = inicializar_pcb(cliente_socket);
                
                //mostrar_lista(pcb->instrucciones); //chequeo para ver si se cargaron bien las instrucciones
                
                // Agregamos el proceso creado a NEW
                pthread_mutex_lock(&mutex_new);
                queue_push(procesos_en_new, pcb);
                pthread_mutex_unlock(&mutex_new);

                log_info(logger, "Se crea el proceso %d en NEW", pcb->pid); //log obligatorio

                // Avisamos que agregamos un nuevo proceso a NEW
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