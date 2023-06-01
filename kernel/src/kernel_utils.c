#include "kernel_utils.h"

t_log* logger;
t_kernel_config* config_kernel;
int pid_counter = 1;

// Colas de los estados de los procesos
t_queue* procesos_en_new;
t_list* procesos_en_ready;
t_pcb* proceso_en_running;
t_list* recursos;


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

int cliente_socket_cpu;


void cargar_config_kernel(t_config* config) {
    
    config_kernel = malloc(sizeof(t_kernel_config));

    // IPs Y PUERTOS
    config_kernel->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    config_kernel->PUERTO_MEMORIA = config_get_int_value(config, "PUERTO_MEMORIA");

    config_kernel->IP_FILESYSTEM = config_get_string_value(config, "IP_FILESYSTEM");
    config_kernel->PUERTO_FILESYSTEM = config_get_int_value(config, "PUERTO_FILESYSTEM");

    config_kernel->IP_CPU = config_get_string_value(config, "IP_CPU");
    config_kernel->PUERTO_CPU = config_get_int_value(config, "PUERTO_CPU");

    config_kernel->PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");

    // PLANIFICACION
    config_kernel->ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    config_kernel->ESTIMACION_INICIAL = config_get_int_value(config, "ESTIMACION_INICIAL");
    config_kernel->HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");
    config_kernel->GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");

    // RECURSOS
    config_kernel->RECURSOS = config_get_array_value(config, "RECURSOS");
    config_kernel->INSTANCIAS_RECURSOS = config_get_array_value(config, "INSTANCIAS_RECURSOS");
    
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


void inicializar_recursos() {
    recursos = list_create();    
    t_recurso* recurso;
    
    int i = 0;
    while(config_kernel->RECURSOS[i] != NULL) {
        recurso = malloc(sizeof(t_recurso));
        
        recurso->dispositivo = config_kernel->RECURSOS[i];
        recurso->cantidad_disponible = atoi(config_kernel->INSTANCIAS_RECURSOS[i]);
        recurso->cola_bloqueados = queue_create();

        list_add(recursos, recurso);

        i++;
    }
}



// TESTING
void falopa1() {
    //int i = 0;

    /*while(recursos[i] != NULL) {
       t_recurso* dd = list_get(recursos, i);
       printf("DISPOSITIVO: %s", dd->dispositivo);
       printf("CANTIDAD: %d", dd->cantidad_disponible);
       
       i++;
    }*/

    t_recurso* dd = list_remove(recursos, 1);
    printf("DISPOSITIVO: %s \n", dd->dispositivo);
    printf("CANTIDAD: %d \n", dd->cantidad_disponible);



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
    
    
    //ACA PODEMOS SACAR ESTE PARAMETRO QUE RECIBE, YA QUE EL SOCKET DE CPU ES GLOBAL -> POR AHORA NO LO SACO PORQUE NO QUIERO ROMPER NADA
    cliente_socket_cpu = (intptr_t) void_cliente_socket;
    
    
    while(1) {
        op_code cod_op = recibir_operacion(cliente_socket_cpu);
        
        // Avisamos que el proceso deja de correr, y que puede ingresar otro en la cpu
        
        switch((int)cod_op) {
            case CONTEXTO_EJECUCION:
            {
                log_info(logger, "Me llego el codigo de operacion CONTEXTO_EJECUCION \n");

                t_contexto_ejecucion* contexto_recibido = malloc(sizeof(t_contexto_ejecucion));
                recv_contexto(cliente_socket_cpu, contexto_recibido);

                log_warning(logger,"PID: %d - PROCESO DESALOJADO \n", contexto_recibido->pid);

                
                // Apenas recibimos el contexto lo reasignamos al PCB que se guardo antes de mandar el proceso a RUNNING
                
                proceso_en_running->pc = contexto_recibido->pc;
                proceso_en_running->registros_cpu = contexto_recibido->registros_cpu;

                // free(contexto_recibido->registros_cpu); // ESTE NO VA PORQUE ESTOY LIBERANDO EL ESPACIO EN MEMORIA EN DONDE ESTAN LOS REGISTROS.
                // list_destroy_and_destroy_elements(contexto_recibido->instrucciones, free);
                // free(contexto_recibido);
                break;
                
            }
            
            case PROCESO_DESALOJADO:
            {
                int motivo_desalojo;
                t_list* lista_parametros_recibida = list_create();
                
                log_info(logger, "Me llego el codigo de operacion PROCESO_DESALOJADO \n");

                recv_desalojo(cliente_socket_cpu, &motivo_desalojo, lista_parametros_recibida);

                // mostrar_lista(lista_parametros_recibida);


                //log_info(logger, "MOTIVO DE DESALOJO: %d \n", motivo_desalojo);


                // Aca deberiamos hacer un switch nuevo para preguntar que se debe hacer segun el motivo que se recibio
                manejar_proceso_desalojado(motivo_desalojo, lista_parametros_recibida);

                
                // El semaforo debe ir al final del case debido a que la variable proceso_en_running debe ser la misma hasta 
                // que vuelva a encolar en ready, en manejar_proceso_desalojado, una vez que hace esto ya puede liberarse la cpu con el semaforo
                // Si el semaforo se pone antes, el planificador (que esta en otro hilo) va a pisar el proceso en running (variable global)
                // Al toque roque al pique enrique
                // sem_post(&cpu_libre); // En algunos casos (Wait) puede que la cpu no se libere para otro proceso. Asi que esto aca creo que no va.

                // sem_post(&maximo_grado_de_multiprogramacion); // ESTO NO VA

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
    switch((int)motivo_desalojo) {
        case YIELD:
        {
            log_info(logger, "Motivo desalojo es YIELD \n");

            volver_a_encolar_en_ready(proceso_en_running);
            
            sem_post(&cpu_libre);
            break;
        }
        case EXIT:
        {
            
            log_info(logger, "Motivo desalojo es EXIT \n");         
            
            matar_proceso("SUCCESS");
            sem_post(&cpu_libre);    
            break;
        }
        case WAIT:
        {
            log_info(logger, "Motivo desalojo es WAIT \n");
            
            char* recurso_solicitado = (char*)list_get(lista_parametros, 0);

            log_info(logger, "RECURSO SOLICITADO ES %s\n", recurso_solicitado);

            t_recurso* recurso = recurso_en_lista(recurso_solicitado);


            if(recurso != NULL) {
                recurso->cantidad_disponible--;
                if(recurso->cantidad_disponible < 0)
                {
                    printf("ME BLOQUEE AYUDAME LOCOOO\n");
                    queue_push(recurso->cola_bloqueados, proceso_en_running);       //mutexito???
                    sem_post(&cpu_libre);
                }
                else{
                    log_info(logger, "Voy a volver a running XD \n");
                    volver_a_running();
                }
            }
            else {
                log_error(logger, "NO ENCONTRE EL RECURSITO");
                matar_proceso("FILE_NOT_FOUND");
            }
            break;
        }
        case SIGNAL:
        {
            log_info(logger, "Motivo desalojo es SIGNAL \n");
            
            char* recurso_solicitado = (char*)list_get(lista_parametros, 0);
            
            log_info(logger, "RECURSO LIBERADO ES %s\n", recurso_solicitado);
            
            t_recurso* recurso = recurso_en_lista(recurso_solicitado);
            
            
            //log_info(logger, "EJECUTE RECURSO EN LISTA \n");
            
            //printf("FALOPA2");  // NO USEN PRINTF PARA DEBUGGEAR PORQUE A VECES NO ANDAN 


            if(recurso != NULL) {
                recurso->cantidad_disponible++;
                log_info(logger, "Cantidad disponible %d", recurso->cantidad_disponible);   // prueba
                 
                if(recurso->cantidad_disponible <= 0){
                   log_info(logger, "Voy a sacar a un proceso de la cola de bloqueados");
                   t_pcb* proceso = queue_pop(recurso->cola_bloqueados);
                   volver_a_encolar_en_ready(proceso);
                }
                volver_a_running();        //devuelve a running el proceso que peticiono el signal
            } 
            else {
                log_error(logger, "NO ENCONTRE EL RECURSITO");
                matar_proceso("FILE_NOT_FOUND");
            }
            break;
        }
        case IO:
        {
            // Proceso se bloquea una cantidad de tiempo, pero no deberia bloquear al resto.
            args_io* argumentos_io = malloc(sizeof(argumentos_io));
            
            argumentos_io->proceso = proceso_en_running;
            argumentos_io->tiempo = atoi((char*)list_get(lista_parametros, 0));

            pthread_t hilo_io;
	        pthread_create(&hilo_io, NULL, (void*)bloquear_proceso, (args_io*) argumentos_io);
	        pthread_detach(hilo_io);

            sem_post(&cpu_libre); // A pesar de que el proceso se bloquee la CPU estará libre, así pueden seguir ejecutando otros procesos.

            break;
        }
    }  
}



void bloquear_proceso(args_io* argumentos_io){
    int tiempo = argumentos_io->tiempo;
    t_pcb* proceso = argumentos_io->proceso;

    log_info(logger, "Proceso %d se bloqueara %d segundos por IO", proceso->pid, tiempo);

    sleep(tiempo);

    log_info(logger, "Proceso %d se ha desbloqueado", proceso->pid);

    volver_a_encolar_en_ready(proceso);
}


t_recurso* recurso_en_lista(char* recurso_solicitado) {
    t_list_iterator* lista_it = list_iterator_create(recursos);

    while (list_iterator_has_next(lista_it)) {
        t_recurso* recurso = (t_recurso*)list_iterator_next(lista_it);
        
        if (strcmp(recurso->dispositivo, recurso_solicitado) == 0) {
            list_iterator_destroy(lista_it);
            return recurso;
        }
    }
    
    list_iterator_destroy(lista_it);
    return NULL;
}

    
/* t_recurso* recurso_en_lista(char* recurso_solicitado) {

    t_recurso* recurso;
    
   t_list_iterator* lista_it = list_iterator_create(recursos);
   
    
    for(int i = 0; i < list_size(recursos) ; i++) {
        
        
        recurso = list_remove(recursos, i);
        printf("FALOPA 1: \n");
        printf("FALOPA 2: \n");


        printf("EL RECURSITO QUE ENCONTRE ES %s", recurso->dispositivo);

        if(strcmp(recurso->dispositivo, recurso_solicitado) == 0) 
        {
            list_iterator_destroy(lista_it);
            return recurso;
        } 

        list_iterator_next(lista_it);

        
    }
    
    list_iterator_destroy(lista_it);
    recurso = NULL;
    return recurso;
}
 */


/* typedef struct {
    void* dispositivoDeseado;
    char* parametroAdicional;
} ComparadorParams;

bool compararDispositivo(void* elemento, void* parametros) {
    t_recurso* recurso = (t_recurso*)elemento;
    ComparadorParams* params = (ComparadorParams*)parametros;
    
    char* dispositivoDeseado = (char*)params->dispositivoDeseado;
    char* parametroAdicional = params->parametroAdicional;
    
    // Realizar la comparación utilizando los parámetros
    
    return strcmp(recurso->dispositivo, dispositivoDeseado) == 0;
}
 */










/* 
Nodo* buscar(Nodo*lista,unsigned unLeg)
{
    Nodo*r=lista;
    while(r!=NULL && r->info.leg!=unLeg)
        r=r->sig;
    return r;
}
 */
