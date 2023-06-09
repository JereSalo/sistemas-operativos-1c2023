#include "kernel_utils.h"

t_log* logger;
t_kernel_config* config_kernel;
int pid_counter = 1;


//Para HRRN vamos a tener un clock global que se inicia cuando prendemos el kernel
t_temporal* temporal;
double tiempo;


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

int server_cpu;


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

void bloquear_proceso(args_io* argumentos_io){
    //sem_post(&cpu_libre); // A pesar de que el proceso se bloquee la CPU estará libre, así pueden seguir ejecutando otros procesos.
    
    //Lo calculamos aca porque el proceso que volvemos a encolar en ready no es proceso_en_running sino otro
    
    argumentos_io->proceso->tiempo_salida_running = (double)temporal_gettime(temporal);
    estimar_proxima_rafaga(argumentos_io->proceso);
    
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

t_pcb* buscar_y_sacar_proceso(t_list* lista ,t_pcb* proceso_a_buscar) {
    t_list_iterator* lista_it = list_iterator_create(lista);

// si lo encuentra, lo saca de la lista y lo devuelve
    while (list_iterator_has_next(lista_it)) {
        t_pcb* proceso = (t_pcb*)list_iterator_next(lista_it);
        
        if (proceso->pid == proceso_a_buscar->pid) {
            list_iterator_destroy(lista_it);
            list_remove_element(lista, proceso);
           
            return proceso;
        }
    }
    
    list_iterator_destroy(lista_it);
    return NULL;
}


void calcular_tasa_de_respuesta(double tiempo_actual) {
    
    t_list_iterator* lista_it = list_iterator_create(procesos_en_ready);

    while(list_iterator_has_next(lista_it)) {
        t_pcb* proceso = (t_pcb*)list_iterator_next(lista_it);
        
        double tiempo_esperando_en_ready = tiempo_actual - proceso->tiempo_llegada_ready;
        
        proceso->tasa_de_respuesta = 1 + (tiempo_esperando_en_ready / proceso->estimacion_prox_rafaga);




        //log_info(logger, "EL TIEMPO ESPERANDO EN READY DEL PROCESO %d es %f \n",proceso->pid, tiempo_esperando_en_ready);
        //log_info(logger, "LA ESTIMACION DE RAFAGA DEL PROCESO %d es %f \n",proceso->pid, proceso->estimacion_prox_rafaga);
        //log_info(logger, "LA TASA DE RESPUESTA DEL PROCESO %d es %f \n \n",proceso->pid, proceso->tasa_de_respuesta);
        

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


void cargar_contexto_de_ejecucion(t_pcb* pcb, t_contexto_ejecucion* contexto) {
    contexto->pid = pcb->pid;
    contexto->pc = pcb->pc;
    contexto->registros_cpu = pcb->registros_cpu;
    contexto->instrucciones = pcb->instrucciones;
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
