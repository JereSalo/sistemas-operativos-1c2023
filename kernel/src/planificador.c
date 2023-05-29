#include "planificador.h"



// ------------------------------ PLANIFICADOR DE LARGO PLAZO ------------------------------ //

// Pasaje de NEW -> READY
void planificador_largo_plazo() {
    while(1) {      
        t_pcb* proceso;
        char pids[100];     // esto es para mostrar la lista de pids en el logger

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
        // Agregamos el PID del proceso que ahora esta en READY a nuestra lista de PIDS
        list_add(lista_pids, string_itoa(proceso->pid));
        pthread_mutex_unlock(&mutex_ready);
        
                
        log_warning(logger,"PID: %d - Estado anterior: NEW - Estado actual: READY \n", proceso->pid); //log obligatorio
        
        log_warning(logger, "Cola Ready %s: [%s] \n", config_kernel->ALGORITMO_PLANIFICACION, lista_a_string(lista_pids, pids));       //log obligatorio


        
        //mostrar_lista(lista_pids);

        // Avisamos que agregamos un nuevo proceso a de READY
        sem_post(&cant_procesos_ready);
    }
}

// ------------------------------ PLANIFICADOR DE CORTITO PLAZO ------------------------------ //

void planificador_corto_plazo(int fd) {
    while(1){
        t_pcb* proceso;
        t_contexto_ejecucion* contexto_de_ejecucion = malloc(sizeof(t_contexto_ejecucion));


        // Sacamos un proceso de ready y lo mandamos a ejecutar
        
        // Verificamos que la lista de ready no este vacia
        sem_wait(&cant_procesos_ready);
        
        // Verificamos que la cpu este libre -> si no lo esta, no podemos mandar a running
        sem_wait(&cpu_libre);
        
        // ESTO ES POR FIFO, MAS ADELANTE ACA TAMBIEN VA A ESTAR EL HRRN
        pthread_mutex_lock(&mutex_ready);
        proceso = list_remove(procesos_en_ready, 0);

        list_remove(lista_pids, 0);     //removemos de la lista de pids al elemento que se saco
        pthread_mutex_unlock(&mutex_ready);

        

        // Ahora lo mandamos a ejecutar
        
        // Lo guardamos en una variable auxiliar
        pthread_mutex_lock(&mutex_running);
        proceso_en_running = proceso;
        pthread_mutex_unlock(&mutex_running);

        // Y ahora le mandamos el contexto de ejecucion a la CPU para ejecutar el proceso
        // Contexto de ejecucion (por ahora) = PID + PC + REGISTROS + INSTRUCCIONES

        cargar_contexto_de_ejecucion(proceso, contexto_de_ejecucion);
        
        log_warning(logger,"PID: %d - Estado anterior: READY - Estado actual: RUNNING \n", proceso->pid); //log obligatorio
 
        send_contexto(fd, contexto_de_ejecucion);

        free(contexto_de_ejecucion);

    }  
}


void cargar_contexto_de_ejecucion(t_pcb* pcb, t_contexto_ejecucion* contexto) {
    contexto->pid = pcb->pid;
    contexto->pc = pcb->pc;
    contexto->registros_cpu = pcb->registros_cpu;
    contexto->instrucciones = pcb->instrucciones;
}
