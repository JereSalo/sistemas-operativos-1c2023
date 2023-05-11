#include "planificador.h"
#include "kernel.h"

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

// ------------------------------ PLANIFICADOR DE LARGO PLAZO ------------------------------ //


// Pasaje de NEW -> READY
void planificador_largo_plazo(void* logger_parametro) {
    
    t_log* logger = (t_log*) logger_parametro;

    while(1) {
        
        t_pcb* proceso;

        // Preguntamos si hay procesos en NEW y por el maximo grado de multiprogramacion
        sem_wait(&cant_procesos_new);
        sem_wait(&maximo_grado_de_multiprogramacion);       //esto se va a liberar cuando un proceso vaya a exit


        // Si esta todo OK obtenemos el proximo proceso a ser mandado a READY
        pthread_mutex_lock(&mutex_new);
        proceso = queue_pop(procesos_en_new);
        pthread_mutex_unlock(&mutex_new);

        // Agregamos el proceso obtenido a READY
        pthread_mutex_lock(&mutex_ready);
        list_add(procesos_en_ready, proceso);
        pthread_mutex_unlock(&mutex_ready);

        //NO FUNCA -> HAY UN PROBLEMA CON EL LOGGER                
        log_info(logger,"PID: %d - Estado anterior: NEW - Estado actual: READY", proceso->pid); //log obligatorio
        //printf("PID: %d - Estado anterior: NEW - Estado actual: READY", proceso->pid);
        
        // Avisamos que agregamos un nuevo proceso a de READY
        sem_post(&cant_procesos_ready);
    }
}


// ------------------------------ PLANIFICADOR DE CORTITO PLAZO ------------------------------ //
/*
void planificador_corto_plazo(){
    
    while(1){

        t_pcb* proceso;
        t_contexto_ejecucion* contexto_de_ejecucion;

        // Sacamos un proceso de ready y lo mandamos a ejecutar
        
        // Verificamos que la lista de ready no este vacia
        sem_wait(&cant_procesos_ready);
        
        // ESTO ES POR FIFO, MAS ADELANTE ACA TAMBIEN VA A ESTAR EL HRRN
        pthread_mutex_lock(&mutex_ready);
        proceso = list_remove(procesos_en_ready, 0);
        pthread_mutex_unlock(&mutex_ready);

        // Ahora lo mandamos a ejecutar
        
        // Lo guardamos en una variable auxiliar
        pthread_mutex_lock(&mutex_running);
        proceso_en_running = proceso;
        pthread_mutex_unlock(&mutex_running);

        // Y ahora le mandamos el contexto de ejecucion a la CPU para ejecutar el proceso
        // Contexto de ejecucion (por ahora) = PID + PC + REGISTROS + INSTRUCCIONES

        cargar_contexto_de_ejecucion(proceso, contexto_de_ejecucion);
        
    }
    
}
*/