#include "planificador.h"

// ------------------------------ PLANIFICADOR DE LARGO PLAZO ------------------------------ //

// Pasaje de NEW -> READY
void planificador_largo_plazo() {
    while(1) {      
        t_pcb* proceso;

        // Preguntamos si hay procesos en NEW y por el maximo grado de multiprogramacion
        sem_wait(&cant_procesos_new);
        sem_wait(&maximo_grado_de_multiprogramacion);       //esto se va a liberar cuando un proceso finalice

        pthread_mutex_lock(&mutex_new);
        proceso = queue_pop(procesos_en_new);
        pthread_mutex_unlock(&mutex_new);
        
        list_add(lista_global_procesos, proceso); //add
        
        //mostrar_lista_global_procesos(lista_global_procesos);
        
        mandar_a_ready(proceso);
        
        log_warning(logger,"PID: %d - Estado anterior: NEW - Estado actual: READY \n", proceso->pid); //LOG CAMBIO DE ESTADO
    }
}

// ------------------------------ PLANIFICADOR DE CORTITO PLAZO ------------------------------ //

// Pasaje de READY -> RUNNING
void planificador_corto_plazo() {
    while(1){
        // Verificamos que la lista de ready no este vacia
        sem_wait(&cant_procesos_ready);
        // Verificamos que la cpu este libre -> si no lo esta, no podemos mandar a running
        sem_wait(&cpu_libre);
        
        // Sacamos elemento de lista de procesos en ready y de lista de pids. Guardamos proceso en proceso_en_running
        switch(config_kernel->ALGORITMO_PLANIFICACION){
            case FIFO:
            {
                pthread_mutex_lock(&mutex_ready);
                proceso_en_running = list_remove(procesos_en_ready, 0);
                list_remove(lista_pids, 0);     //removemos de la lista de pids al elemento que se saco
                pthread_mutex_unlock(&mutex_ready);
                break;
            }
            case HRRN:
            {
                calcular_tasa_de_respuesta(); // Lo calcula para todos los procesos y lo guarda en la variable tasa_de_respuesta del pcb de cada proceso.
            
                t_pcb* proceso_siguiente_a_running = proceso_con_mayor_tasa_de_respuesta();

                bool coincide_con_pid(void* elemento){  // Pongo esta funcion afuera del mutex para que la seccion critica sea chiquita.
                    return *(int*)elemento == proceso_en_running->pid;
                }
                
                pthread_mutex_lock(&mutex_ready);
                proceso_en_running = buscar_y_sacar_proceso(procesos_en_ready, proceso_siguiente_a_running);
                // Recorrer la lista y por cada elemento ver si el contenido es igual a contenido, si es así hago un list_remove con el indice.
                list_remove_by_condition(lista_pids, coincide_con_pid);
                pthread_mutex_unlock(&mutex_ready);

                proceso_en_running->tiempo_llegada_running = (double)temporal_gettime(temporal); // aca el proceso empieza a ejecutar
                break;
            }
            default:
            {
                log_error(logger, "ALGORITMO DE PLANIFICACION INCORRECTO");
                break;
            }
        }

        mandar_a_running(proceso_en_running);
        
        log_warning(logger,"PID: %d - Estado anterior: READY - Estado actual: RUNNING \n", proceso_en_running->pid); //LOG CAMBIO DE ESTADO
    }  
}

void mandar_a_running(t_pcb* proceso){
    t_contexto_ejecucion* contexto_de_ejecucion = cargar_contexto(proceso);

    send_contexto(server_cpu, contexto_de_ejecucion);

    liberar_contexto(contexto_de_ejecucion);
}

void volver_a_running() {
    mandar_a_running(proceso_en_running);
    
    log_info(logger, "Proceso %d vuelve a Running despues de haber sido desalojado", proceso_en_running->pid);
}

void mandar_a_ready(t_pcb* proceso) {
    proceso->tiempo_llegada_ready = (double)temporal_gettime(temporal);
    pthread_mutex_lock(&mutex_ready);
    
    list_add(procesos_en_ready, proceso);
    
    // Agregamos el PID del proceso que ahora esta en READY a nuestra lista de PIDS
    
    list_add(lista_pids, &proceso->pid);

    char pids[100];
    log_warning(logger, "Cola Ready %s: [%s] \n", config_get_string_value(config, "ALGORITMO_PLANIFICACION"), lista_pids_a_string(lista_pids, pids));   //LOG INGRESO A READY    


    pthread_mutex_unlock(&mutex_ready);
    
    
    char pids_2[100];
    log_warning(logger, "Cola Ready %s: [%s] \n", config_get_string_value(config, "ALGORITMO_PLANIFICACION"), lista_pids_a_string(lista_pids, pids_2));   //LOG INGRESO A READY    

    // Avisamos que agregamos un nuevo proceso a READY
    sem_post(&cant_procesos_ready);

}


