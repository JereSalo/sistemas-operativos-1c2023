#include "planificador.h"
#include "kernel.h"


// Colas de los estados de los procesos aaa
t_queue* procesos_en_new;
t_list* procesos_en_ready;


// Semaforos
pthread_mutex_t mutex_new;
pthread_mutex_t mutex_ready;
sem_t maximo_grado_de_multiprogramacion;
sem_t cant_procesos_new;
sem_t cant_procesos_ready;

// ------------------------------ PLANIFICADOR DE LARGO PLAZO ------------------------------ //


// Pasaje de NEW -> READY
void planificador_largo_plazo() {
    
    while(1) {
        
        t_pcb* proceso;

        // Preguntamos si hay procesos en NEW y por el maximo grado de multiprogramacion
        sem_wait(&cant_procesos_new);
        sem_wait(&maximo_grado_de_multiprogramacion);       //esto se va a liberar cuando un proceso vaya a exit


        // Si esta todo OK obtenemos el proximo proceso a ser mandado a READY
        pthread_mutex_lock(&mutex_new);
        proceso = queue_pop(procesos_en_new);
        pthread_mutex_unlock(&mutex_new);
        
        //printf("%d", proceso->pid);

        //NO FUNCA -> HAY UN PROBLEMA CON EL LOGGER
        //log_info(logger,"PID: %d - Estado anterior: NEW - Estado actual: READY", proceso->pid); //log obligatorio

        printf("PID: %d - Estado anterior: NEW - Estado actual: READY", proceso->pid);


        // Agregamos el proceso obtenido a READY
        pthread_mutex_lock(&mutex_ready);
        list_add(procesos_en_ready, proceso);
        pthread_mutex_unlock(&mutex_ready);

        
        // Avisamos que agregamos un nuevo proceso a de READY
        sem_post(&cant_procesos_ready);
    }
}


