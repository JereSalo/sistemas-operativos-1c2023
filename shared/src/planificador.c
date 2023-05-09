#include "planificador.h"
#include "kernel.h"

// Colas de los estados de los procesos
////t_queue* procesos_en_new;
//t_list* procesos_en_ready;



// ------------------------------ PLANIFICADOR DE LARGO PLAZO ------------------------------ //


// Pasaje de NEW -> READY
void planificador_largo_plazo() {

    //semaforos que necesitamos:
        // mutex de la cola de new
        // mutex de la lista de ready
        // contador del max grado de multiprogramacion (que no este llena)
        // contador de la cola de new (para chequear que no este vacia)
    
    
    //pregunta el grado de multiprogramacion y si hay procesos en new -> con semaforos

    //wait(hay elementos en new)
    //wait(grado de multiprogramacion)

    //si esta todo ok hago un pop de la cola de new
    /* 
    wait(mutex_new);
    pop(cola_new, pcb);
    signal(mutex_new);


    //despues hago un list add a ready

    wait(mutex_ready);
    push(cola_ready, pcb);
    signal(mutex_ready);
    */

    // sem_init(&mutex, 0, 1);

    //sem_init(&sem_contador, 0, grado_multiprogramacion);

    
    // int   pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *); mutex
    // int   sem_init(sem_t *, int, unsigned int); semaphore
}


