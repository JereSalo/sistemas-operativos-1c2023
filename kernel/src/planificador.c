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
        proceso->tiempo_llegada_ready = (double)temporal_gettime(temporal);

        // Agregamos el proceso obtenido a READY
        pthread_mutex_lock(&mutex_ready);
        list_add(procesos_en_ready, proceso);
        int* pid = malloc(sizeof(int)); *pid = proceso->pid;
        list_add(lista_pids, pid);
        pthread_mutex_unlock(&mutex_ready);
        // Agregamos el PID del proceso que ahora esta en READY a nuestra lista de PIDS
        
        //pthread_mutex_lock(&mutex_pids);
        //pthread_mutex_unlock(&mutex_pids);
                
        log_warning(logger,"PID: %d - Estado anterior: NEW - Estado actual: READY \n", proceso->pid); //log obligatorio
        
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
    log_warning(logger, "Finaliza el proceso %d - Motivo: %s \n", proceso_en_running->pid, motivo);       //log obligatorio 
    
    int socket_consola = proceso_en_running->socket_consola;
    int pid_finalizado = proceso_en_running->pid;

    list_destroy_and_destroy_elements(proceso_en_running->instrucciones, free);
    free(proceso_en_running->registros_cpu);
    free(proceso_en_running); // lo mata

    // Avisarle a consola que finalizó el proceso.
    SEND_INT(socket_consola, pid_finalizado);
    send_string(socket_consola, motivo);
    

    
    sem_post(&maximo_grado_de_multiprogramacion);
    sem_post(&cpu_libre);
}


// ------------------------------ PLANIFICADOR DE CORTITO PLAZO ------------------------------ //

// Pasaje de READY -> RUNNING
void planificador_corto_plazo(int fd) {
    while(1){
        t_contexto_ejecucion* contexto_de_ejecucion = malloc(sizeof(t_contexto_ejecucion));
        
        // Verificamos que la lista de ready no este vacia
        sem_wait(&cant_procesos_ready);
        
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
            
            double tiempo_actual = (double)temporal_gettime(temporal);
            
            calcular_tasa_de_respuesta(tiempo_actual);
            
            t_pcb* proceso_siguiente_a_running = proceso_con_mayor_tasa_de_respuesta();
            
            pthread_mutex_lock(&mutex_ready);
            proceso_en_running = buscar_y_sacar_proceso(procesos_en_ready, proceso_siguiente_a_running);


            int contenido = proceso_en_running->pid;
            bool coincide_con_contenido(void* elemento){
                return *(int*)elemento == contenido;
            }
            // Recorrer la lista y por cada elemento ver si el contenido es igual a contenido, si es así hago un list_remove con el indice.
	        list_remove_and_destroy_by_condition(lista_pids, coincide_con_contenido, free);

            
            pthread_mutex_unlock(&mutex_ready);
        }
        else{
            log_error(logger, "ALGORITMO DE PLANIFICACION INCORRECTO");
        }


        // Y ahora le mandamos el contexto de ejecucion a la CPU para ejecutar el proceso
        // Contexto de ejecucion (por ahora) = PID + PC + REGISTROS + INSTRUCCIONES

        cargar_contexto_de_ejecucion(proceso_en_running, contexto_de_ejecucion);
        
        proceso_en_running->tiempo_llegada_running = (double)temporal_gettime(temporal); // aca el proceso empieza a ejecutar
        send_contexto(fd, contexto_de_ejecucion);
        
        log_warning(logger,"PID: %d - Estado anterior: READY - Estado actual: RUNNING \n", proceso_en_running->pid); //log obligatorio
 
        free(contexto_de_ejecucion);
    }  
}





void calcular_tasa_de_respuesta(double tiempo_actual) {
    
    t_list_iterator* lista_it = list_iterator_create(procesos_en_ready);

    while(list_iterator_has_next(lista_it)) {
        t_pcb* proceso = (t_pcb*)list_iterator_next(lista_it);
        
        double tiempo_esperando_en_ready = tiempo_actual - proceso->tiempo_llegada_ready;
        
        proceso->tasa_de_respuesta = 1 + (tiempo_esperando_en_ready / proceso->estimacion_prox_rafaga);

        log_info(logger, "EL TIEMPO ESPERANDO EN READY DEL PROCESO %d es %f \n",proceso->pid, tiempo_esperando_en_ready);
        log_info(logger, "LA ESTIMACION DE RAFAGA DEL PROCESO %d es %f \n",proceso->pid, proceso->estimacion_prox_rafaga);
        log_info(logger, "LA TASA DE RESPUESTA DEL PROCESO %d es %f \n \n",proceso->pid, proceso->tasa_de_respuesta);
        

    }

    list_iterator_destroy(lista_it);
}


void estimar_proxima_rafaga(t_pcb* proceso) {

    double alpha = config_kernel->HRRN_ALFA;
    double tiempo_rafaga_actual = (proceso->tiempo_salida_running) - (proceso->tiempo_llegada_running);               
    double estimacion_actual = proceso->estimacion_prox_rafaga;
        
    // ESTE CALCULO TENEMOS QUE HACERLO CUANDO UN PROCESO FINALIZA SU EJECUCION
    proceso->estimacion_prox_rafaga = (alpha * tiempo_rafaga_actual) + ((1 - alpha) * estimacion_actual);
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


    send_contexto(server_cpu, contexto_de_ejecucion);

    // log_warning(logger,"PID: %d - Estado anterior: READY - Estado actual: RUNNING \n", proceso_en_running->pid); // Este log para mi esta mal.
    log_info(logger, "Proceso %d vuelve a Running despues de haber sido desalojado", proceso_en_running->pid);

    free(contexto_de_ejecucion);
}




void volver_a_encolar_en_ready (t_pcb* proceso) {
    // Aca debemos preguntar por el algoritmo y replanificar segun corresponda
    // Como todavia no hicimos HRRN lo hago por FIFO

    // Ya tenemos el PCB con el contexto modificado (case anterior)

    // Agregamos el proceso obtenido a READY
    
    proceso->tiempo_llegada_ready = (double)temporal_gettime(temporal);
    pthread_mutex_lock(&mutex_ready);
    list_add(procesos_en_ready, proceso);
    
    // Agregamos el PID del proceso que ahora esta en READY a nuestra lista de PIDS
    int* pid = malloc(sizeof(int)); *pid = proceso->pid;
    list_add(lista_pids, pid);
    pthread_mutex_unlock(&mutex_ready);
    

    log_warning(logger, "Cola Ready %s: [%s] \n", config_kernel->ALGORITMO_PLANIFICACION, lista_pids_a_string(lista_pids, pids));    

    //mostrar_lista(lista_pids);

    // Avisamos que agregamos un nuevo proceso a READY
    sem_post(&cant_procesos_ready);
    
    //proceso_en_running->tiempo_salida_running = time(NULL);
}


void cargar_contexto_de_ejecucion(t_pcb* pcb, t_contexto_ejecucion* contexto) {
    contexto->pid = pcb->pid;
    contexto->pc = pcb->pc;
    contexto->registros_cpu = pcb->registros_cpu;
    contexto->instrucciones = pcb->instrucciones;
}


