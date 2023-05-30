#include "kernel_utils.h"

t_log* logger;
t_kernel_config* config_kernel;
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
sem_t cpu_libre;
pthread_mutex_t mutex_pids;
t_list* lista_pids;

void cargar_config_kernel(t_config* config) {
    
    config_kernel = malloc(sizeof(t_kernel_config));

    
    config_kernel->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    config_kernel->PUERTO_MEMORIA = config_get_int_value(config, "PUERTO_MEMORIA");

    config_kernel->IP_FILESYSTEM = config_get_string_value(config, "IP_FILESYSTEM");
    config_kernel->PUERTO_FILESYSTEM = config_get_int_value(config, "PUERTO_FILESYSTEM");

    config_kernel->IP_CPU = config_get_string_value(config, "IP_CPU");
    config_kernel->PUERTO_CPU = config_get_int_value(config, "PUERTO_CPU");

    config_kernel->PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");

    config_kernel->ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    
    
    config_kernel->ESTIMACION_INICIAL = config_get_int_value(config, "ESTIMACION_INICIAL");

    config_kernel->HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");

    config_kernel->GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");

    //FALTAN LAS DOS LISTAS


    //log_info(logger, "Config cargada en config_kernel \n");
}

void inicializar_semaforos() {
    pthread_mutex_init(&mutex_new, NULL);
    pthread_mutex_init(&mutex_ready, NULL);
    pthread_mutex_init(&mutex_running, NULL);
    pthread_mutex_init(&mutex_pids, NULL);

    //sem_init(&sem_proceso_en_running, 0, 0);
    sem_init(&cant_procesos_new, 0, 0);
    sem_init(&cant_procesos_ready, 0, 0);
    sem_init(&maximo_grado_de_multiprogramacion, 0, config_kernel->GRADO_MAX_MULTIPROGRAMACION);

    sem_init(&cpu_libre, 0, 1);

    //log_info(logger, "Semaforos inicializados \n");
}

void inicializar_colas() {
    procesos_en_new = queue_create();
    procesos_en_ready = list_create();
    lista_pids = list_create();
}

void inicializar_registros(t_registros_cpu* registros) {        
    asignar_a_registro("AX", "0000", registros);
    asignar_a_registro("BX", "0000", registros);
    asignar_a_registro("CX", "0000", registros);
    asignar_a_registro("DX", "0000", registros);

    asignar_a_registro("EAX", "00000000", registros);
    asignar_a_registro("EBX", "00000000", registros);
    asignar_a_registro("ECX", "00000000", registros);
    asignar_a_registro("EDX", "00000000", registros);
    
    asignar_a_registro("RAX", "0000000000000000", registros);
    asignar_a_registro("RBX", "0000000000000000", registros);
    asignar_a_registro("RCX", "0000000000000000", registros);
    asignar_a_registro("RDX", "0000000000000000", registros);
    
}


t_pcb* inicializar_pcb(int cliente_socket) {
    
    // Recibimos las instrucciones
    t_list* instrucciones_recibidas = list_create();

    if(!recv_instrucciones(cliente_socket, instrucciones_recibidas)){
        log_error(logger, "Fallo recibiendo INSTRUCCIONES \n");
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
    inicializar_registros(pcb->registros_cpu);
    pcb->tabla_segmentos = list_create();                               //TODO: la dejamos como vacia pero la tabla la va a armar la memoria
    pcb->estimacion_prox_rafaga = config_kernel->ESTIMACION_INICIAL;            
    pcb->tiempo_llegada_ready = 0;                                      //TODO: Esto lo tenemos que cambiar por el timestamp
    pcb->tabla_archivos_abiertos = list_create();

    return pcb;
}

void procesar_consola(void* void_cliente_socket) {
    
    int cliente_socket = (intptr_t) void_cliente_socket;
    
    while(1) {
        op_code cod_op = recibir_operacion(cliente_socket);
        t_pcb* pcb;

        switch((int)cod_op) {
            case INSTRUCCIONES:
            {
                log_info(logger, "Me llego el codigo de operacion INSTRUCCIONES \n");

                // Inicializamos el PCB de un proceso (esto implica crearlo)
                pcb = inicializar_pcb(cliente_socket);
                
                // Agregamos el proceso creado a NEW
                pthread_mutex_lock(&mutex_new);
                queue_push(procesos_en_new, pcb);
                pthread_mutex_unlock(&mutex_new);

                log_warning(logger, "Se crea el proceso %d en NEW \n", pcb->pid); //log obligatorio

                // Avisamos que agregamos un nuevo proceso a NEW
                sem_post(&cant_procesos_new);   
                
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



void procesar_cpu(void* void_cliente_socket) {
    
    int cliente_socket = (intptr_t) void_cliente_socket;
    
    
    
    while(1) {
        op_code cod_op = recibir_operacion(cliente_socket);
        
        t_contexto_ejecucion* contexto_recibido = malloc(sizeof(t_contexto_ejecucion));

        int motivo_desalojo;
        t_list* lista_parametros = list_create();
        
        // Avisamos que el proceso deja de correr, y que puede ingresar otro en la cpu
        
        
        switch((int)cod_op) {
            
            case CONTEXTO_EJECUCION:
            {
                log_info(logger, "Me llego el codigo de operacion CONTEXTO_EJECUCION \n");
                
                recv_contexto(cliente_socket, contexto_recibido);

                log_warning(logger,"PID: %d - PROCESO DESALOJADO \n", contexto_recibido->pid);

                
                
                // Apenas recibimos el contexto lo reasignamos al PCB que se guardo antes de mandar el proceso a RUNNING
                
                proceso_en_running->pc = contexto_recibido->pc;
                proceso_en_running->registros_cpu = contexto_recibido->registros_cpu;

                free(contexto_recibido->registros_cpu);
                list_destroy_and_destroy_elements(contexto_recibido->instrucciones, free);
                free(contexto_recibido);
                break;
                
            }
            
            case PROCESO_DESALOJADO:
            {
                
                log_info(logger, "Me llego el codigo de operacion PROCESO_DESALOJADO \n");

                recv_desalojo(cliente_socket, &motivo_desalojo, lista_parametros);

                //log_info(logger, "MOTIVO DE DESALOJO: %d \n", motivo_desalojo);


                // Aca deberiamos hacer un switch nuevo para preguntar que se debe hacer segun el motivo que se recibio
                manejar_proceso_desalojado(motivo_desalojo, lista_parametros);

                
                // El semaforo debe ir al final del case debido a que la variable proceso_en_running debe ser la misma hasta 
                // que vuelva a encolar en ready, en manejar_proceso_desalojado, una vez que hace esto ya puede liberarse la cpu con el semaforo
                // Si el semaforo se pone antes, el planificador (que esta en otro hilo) va a pisar el proceso en running (variable global)
                // Al toque roque al pique enrique
                sem_post(&cpu_libre);
                sem_post(&maximo_grado_de_multiprogramacion);

                break;
            }
            case -1:
            {
			    log_error(logger, "El cliente CPU se desconecto. Terminando Servidor \n");
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

void manejar_proceso_desalojado(op_instruccion motivo_desalojo, t_list* lista_parametros) {
   
    switch(motivo_desalojo) {
        case YIELD:
        {
            log_info(logger, "Motivo desalojo es YIELD \n");         
            volver_a_encolar_en_ready();         
                                   
            break;
        }
        case EXIT:
        {
                
            log_info(logger, "Motivo desalojo es EXIT \n");         
            // Avisamos que ya puede entrar otro proceso a memoria principal
            sem_post(&maximo_grado_de_multiprogramacion);
            
            matar_proceso();
            
            break;
        }
                   
        //mostrar_lista(lista_parametros);   
    }  
}

    










                /*switch(motivo_desalojo) {
                    case YIELD:{
                        log_info(logger, "Motivo desalojo es YIELD \n");         
                        
                        
                        
                        // Aca debemos preguntar por el algoritmo y replanificar segun corresponda
                        // Como todavia no hicimos HRRN lo hago por FIFO

                        // Ya tenemos el PCB con el contexto modificado (case anterior)

                        
                        //sem_wait(&maximo_grado_de_multiprogramacion);       //esto se va a liberar cuando un proceso vaya a exit
                        
                        // Agregamos el proceso obtenido a READY
                        //pthread_mutex_lock(&mutex_ready);
                        //list_add(procesos_en_ready, proceso_en_running);
                        
                        // Agregamos el PID del proceso que ahora esta en READY a nuestra lista de PIDS
                        //list_add(lista_pids, string_itoa(proceso_en_running->pid));
                        //pthread_mutex_unlock(&mutex_ready);


                        //mostrar_lista(lista_pids);

                        // Avisamos que agregamos un nuevo proceso a READY
                        //sem_post(&cant_procesos_ready);
                        break;

                    }
                    case EXIT:{
                            
                        log_info(logger, "Motivo desalojo es EXIT \n");         
                        // Avisamos que ya puede entrar otro proceso a memoria principal
                        //sem_post(&maximo_grado_de_multiprogramacion);
                        
                        log_warning(logger, "Finaliza el proceso %d - Motivo: SUCCESS \n", contexto_recibido->pid);       //log obligatorio 
                        break;
                    }
                }
                
                //mostrar_lista(lista_parametros); 
                

            }
            
        }
    }
}*/