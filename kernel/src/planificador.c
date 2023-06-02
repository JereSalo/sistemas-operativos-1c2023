#include "planificador.h"

char pids[100];

// ------------------------------ PLANIFICADOR DE LARGO PLAZO ------------------------------ //

// Pasaje de NEW -> READY
void planificador_largo_plazo() {
    while(1) {      
        t_pcb* proceso;
        //char pids[100];     // esto es para mostrar la lista de pids en el logger

        // Preguntamos si hay procesos en NEW y por el maximo grado de multiprogramacion
        sem_wait(&cant_procesos_new);
        sem_wait(&maximo_grado_de_multiprogramacion);       //esto se va a liberar cuando un proceso vaya a exit

        // Si esta todo OK obtenemos el proximo proceso a ser mandado a READY
        pthread_mutex_lock(&mutex_new);
        proceso = queue_pop(procesos_en_new);
        pthread_mutex_unlock(&mutex_new);

        // Asignamos el tiempo de llegada a ready -> en este instante se manda a ready (Importante para HRRN)
        proceso->tiempo_llegada_ready = time(NULL);

        // Agregamos el proceso obtenido a READY
        pthread_mutex_lock(&mutex_ready);
        list_add(procesos_en_ready, proceso);
        list_add(lista_pids, proceso->pid);
        pthread_mutex_unlock(&mutex_ready);
        // Agregamos el PID del proceso que ahora esta en READY a nuestra lista de PIDS
        
        //pthread_mutex_lock(&mutex_pids);
        //pthread_mutex_unlock(&mutex_pids);
                
        log_warning(logger,"PID: %d - Estado anterior: NEW - Estado actual: READY \n", *proceso->pid); //log obligatorio
        
        log_warning(logger, "Cola Ready %s: [%s] \n", config_kernel->ALGORITMO_PLANIFICACION, lista_pids_a_string(lista_pids, pids));       //log obligatorio


        //muestro la lista de pids para debuggear porque hay un problema
        //cuando deja de correr la cpu no se encolan bien las cosas a la cola de ready        
        //mostrar_lista(lista_pids);

        // Avisamos que agregamos un nuevo proceso a de READY
        sem_post(&cant_procesos_ready);
    }
}

void matar_proceso(char* motivo) {
    // Aca no necesariamente el motivo es success...
    log_warning(logger, "Finaliza el proceso %d - Motivo: %s \n", *proceso_en_running->pid, motivo);       //log obligatorio 
    
    int socket_consola = proceso_en_running->socket_consola;

    list_destroy_and_destroy_elements(proceso_en_running->instrucciones, free);
    free(proceso_en_running->registros_cpu);
    free(proceso_en_running); // lo mata

    // Avisarle a consola que finalizó el proceso.
    send_string(socket_consola, motivo);

    
    sem_post(&maximo_grado_de_multiprogramacion);
    sem_post(&cpu_libre);
}


// ------------------------------ PLANIFICADOR DE CORTITO PLAZO ------------------------------ //

// Pasaje de READY -> RUNNING
void planificador_corto_plazo(int fd) {
    while(1){
        //t_pcb* proceso;
        t_contexto_ejecucion* contexto_de_ejecucion = malloc(sizeof(t_contexto_ejecucion));

        // int valor;

        // Sacamos un proceso de ready y lo mandamos a ejecutar
        
        // Verificamos que la lista de ready no este vacia
        sem_wait(&cant_procesos_ready);
        
        // sem_getvalue(&cpu_libre, &valor);
        // printf("LA CPU TIENE %d PROCESOS\n", valor);

        // Verificamos que la cpu este libre -> si no lo esta, no podemos mandar a running
        sem_wait(&cpu_libre);
        
        
        // ------------ FIFITO ----------- //

        if(strcmp(config_kernel->ALGORITMO_PLANIFICACION, "FIFO") == 0){
            pthread_mutex_lock(&mutex_ready);
            proceso_en_running = list_remove(procesos_en_ready, 0);
            list_remove(lista_pids, 0);     //removemos de la lista de pids al elemento que se saco
            pthread_mutex_unlock(&mutex_ready);
        }

        // ------------ HRRNCITO ----------- //

        else if(strcmp(config_kernel->ALGORITMO_PLANIFICACION, "HRRN") == 0){
            double tiempo_actual = time(NULL); 
            calcular_tasa_de_respuesta(tiempo_actual);
            t_pcb* proceso_siguiente_a_running = proceso_con_mayor_tasa_de_respuesta();
            // log_info(logger, "Proceso siguiente a running PID: %d", *proceso_siguiente_a_running->pid); // DEBUG
            pthread_mutex_lock(&mutex_ready);
            proceso_en_running = buscar_y_sacar_proceso(procesos_en_ready, proceso_siguiente_a_running);
            // log_info(logger, "Proceso en running PID: %d", *proceso_en_running->pid); // DEBUG
            if(!list_remove_element(lista_pids, proceso_en_running->pid)){log_error(logger, "No encontre proceso pid %d", *proceso_en_running->pid);}
            pthread_mutex_unlock(&mutex_ready);
        }
        else{
            log_error(logger, "ALGORITMO DE PLANIFICACION INCORRECTO");
        }


        // Y ahora le mandamos el contexto de ejecucion a la CPU para ejecutar el proceso
        // Contexto de ejecucion (por ahora) = PID + PC + REGISTROS + INSTRUCCIONES

        cargar_contexto_de_ejecucion(proceso_en_running, contexto_de_ejecucion);
        
        proceso_en_running->tiempo_llegada_running = time(NULL); // aca el proceso empieza a ejecutar
        send_contexto(fd, contexto_de_ejecucion);
        
        log_warning(logger,"PID: %d - Estado anterior: READY - Estado actual: RUNNING \n", *proceso_en_running->pid); //log obligatorio
 
        free(contexto_de_ejecucion);
    }  
}

void calcular_tasa_de_respuesta(double tiempo_actual) {
    
    t_list_iterator* lista_it = list_iterator_create(procesos_en_ready);

    while(list_iterator_has_next(lista_it)) {
        t_pcb* proceso = (t_pcb*)list_iterator_next(lista_it);
        
        double tiempo_rafaga_actual = proceso->tiempo_salida_running - proceso->tiempo_llegada_running;
        double tiempo_esperando_en_ready = tiempo_actual - proceso->tiempo_llegada_ready;
        proceso->estimacion_prox_rafaga = config_kernel->HRRN_ALFA * tiempo_rafaga_actual + (1 - config_kernel->HRRN_ALFA) + proceso->estimacion_prox_rafaga;
        proceso->tasa_de_respuesta = 1 + tiempo_esperando_en_ready / proceso->estimacion_prox_rafaga;

        // TASITA DE RESPUESTA = (tiempo esperando en ready + estimado rafaga) / estimado rafaga
        // ESTIMACION_SIGUIENTE = HRRN_ALFA * TE (tiempo ejecucion de la rafaga actual) + (1 - HRRN_ALFA) * ESTIMACION_INICIAL
        // TE = TRUN FINAL - TRUN INICIAL
    }

    list_iterator_destroy(lista_it);
}


t_pcb* proceso_con_mayor_tasa_de_respuesta() {

    t_list_iterator* lista_it = list_iterator_create(procesos_en_ready);

    double mayor_tasa = 0;
    t_pcb* proceso_tasa = malloc(sizeof(t_pcb));  //No sabemos si hay que pedir memoria

    while(list_iterator_has_next(lista_it)) {
        t_pcb* proceso = (t_pcb*)list_iterator_next(lista_it);

        if(proceso->tasa_de_respuesta > mayor_tasa) {
            mayor_tasa = proceso->tasa_de_respuesta;
            proceso_tasa = proceso;
        }
    }
    list_iterator_destroy(lista_it);
    return proceso_tasa;
}


void volver_a_running() {
        
    t_contexto_ejecucion* contexto_de_ejecucion = malloc(sizeof(t_contexto_ejecucion));

    cargar_contexto_de_ejecucion(proceso_en_running, contexto_de_ejecucion);


    send_contexto(cliente_socket_cpu, contexto_de_ejecucion);

    // log_warning(logger,"PID: %d - Estado anterior: READY - Estado actual: RUNNING \n", proceso_en_running->pid); // Este log para mi esta mal.
    log_info(logger, "Proceso %d vuelve a Running despues de haber sido desalojado", *proceso_en_running->pid);

    free(contexto_de_ejecucion);
}




void volver_a_encolar_en_ready (t_pcb* proceso) {
    // Aca debemos preguntar por el algoritmo y replanificar segun corresponda
    // Como todavia no hicimos HRRN lo hago por FIFO

    // Ya tenemos el PCB con el contexto modificado (case anterior)

    // Agregamos el proceso obtenido a READY
    
    proceso->tiempo_llegada_ready = time(NULL);
    pthread_mutex_lock(&mutex_ready);
    list_add(procesos_en_ready, proceso);
    
    // Agregamos el PID del proceso que ahora esta en READY a nuestra lista de PIDS
    list_add(lista_pids, proceso->pid);
    pthread_mutex_unlock(&mutex_ready);
    

    log_warning(logger, "Cola Ready %s: [%s] \n", config_kernel->ALGORITMO_PLANIFICACION, lista_pids_a_string(lista_pids, pids));    

    //mostrar_lista(lista_pids);

    // Avisamos que agregamos un nuevo proceso a READY
    sem_post(&cant_procesos_ready);
    
    proceso_en_running->tiempo_salida_running = time(NULL);
}


void cargar_contexto_de_ejecucion(t_pcb* pcb, t_contexto_ejecucion* contexto) {
    contexto->pid = pcb->pid;
    contexto->pc = pcb->pc;
    contexto->registros_cpu = pcb->registros_cpu;
    contexto->instrucciones = pcb->instrucciones;
}


