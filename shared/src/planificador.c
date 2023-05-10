#include "planificador.h"
#include "kernel.h"

// Colas de los estados de los procesos
t_queue* procesos_en_new;
t_list* procesos_en_ready;


// Semaforos
pthread_mutex_t mutex_new;                  // mutex de la cola de new
pthread_mutex_t mutex_ready;                // mutex de la lista de ready
sem_t maximo_grado_de_multiprogramacion;    // contador del max grado de multiprogramacion (que no este llena)
sem_t cant_procesos_new;                    // contador de la cola de new (para chequear que no este vacia)
sem_t cant_procesos_ready;                  // contador de la cola de ready (para chequear que no este vacia)

// ------------------------------ PLANIFICADOR DE LARGO PLAZO ------------------------------ //


// Pasaje de NEW -> READY
void planificador_largo_plazo() {

    t_pcb* proceso;

    //pregunta el grado de multiprogramacion y si hay procesos en new -> con semaforos
    
    sem_wait(&cant_procesos_new);
    sem_wait(&maximo_grado_de_multiprogramacion);       //esto se va a liberar cuando un proceso vaya a exit

    //si esta todo ok hago un pop de la cola de new
    
    pthread_mutex_lock(&mutex_new);
    proceso = queue_pop(procesos_en_new);
    pthread_mutex_unlock(&mutex_new);

    
    printf("Saque un proceso de la cola de new, con ID %d", proceso->pid); // usar loggers en vez de printf

    
    //despues hago un list add a ready

    pthread_mutex_lock(&mutex_ready);
    list_add(procesos_en_ready, proceso);
    pthread_mutex_unlock(&mutex_ready);

    printf("AGREGAMOS UN PROCESO A READY"); // usar logger
    
    // hacemos un signal de un semaforo de cant_procesos_en_ready (para el planificador de corto plazo)
    sem_post(&cant_procesos_ready);

}


