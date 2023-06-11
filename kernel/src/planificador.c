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
        
        mandar_a_ready(proceso);
        log_warning(logger,"PID: %d - Estado anterior: NEW - Estado actual: READY \n", proceso->pid); //log obligatorio
    }
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
                list_remove_and_destroy_by_condition(lista_pids, coincide_con_pid, free);

                pthread_mutex_unlock(&mutex_ready);
                break;
            }
            default:
            {
                log_error(logger, "ALGORITMO DE PLANIFICACION INCORRECTO");
                break;
            }
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

void volver_a_running() {
        
    t_contexto_ejecucion* contexto_de_ejecucion = malloc(sizeof(t_contexto_ejecucion));

    cargar_contexto_de_ejecucion(proceso_en_running, contexto_de_ejecucion);


    send_contexto(server_cpu, contexto_de_ejecucion);

    // log_warning(logger,"PID: %d - Estado anterior: READY - Estado actual: RUNNING \n", proceso_en_running->pid); // Este log para mi esta mal.
    log_info(logger, "Proceso %d vuelve a Running despues de haber sido desalojado", proceso_en_running->pid);

    free(contexto_de_ejecucion);
}


void mandar_a_ready(t_pcb* proceso) {
    proceso->tiempo_llegada_ready = (double)temporal_gettime(temporal);
    pthread_mutex_lock(&mutex_ready);
    list_add(procesos_en_ready, proceso);
    
    // Agregamos el PID del proceso que ahora esta en READY a nuestra lista de PIDS
    int* pid = malloc(sizeof(int)); *pid = proceso->pid;
    list_add(lista_pids, pid);
    pthread_mutex_unlock(&mutex_ready);
    
    char pids[100];
    log_warning(logger, "Cola Ready %s: [%s] \n", config_get_string_value(config, "ALGORITMO_PLANIFICACION"), lista_pids_a_string(lista_pids, pids));    

    // Avisamos que agregamos un nuevo proceso a READY
    sem_post(&cant_procesos_ready);
}


