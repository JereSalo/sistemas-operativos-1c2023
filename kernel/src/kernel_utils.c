#include "kernel_utils.h"

// ------------------------------ VARIABLES GLOBALES ------------------------------ //

// LOGGER Y CONFIG
t_log* logger;
t_config* config;
t_kernel_config* config_kernel;

// HRRN -> vamos a tener un clock global que se inicia cuando prendemos el kernel
t_temporal* temporal;
double tiempo;

// COLAS DE ESTADOS DE PROCESOS
t_queue* procesos_en_new;
t_list* procesos_en_ready;
t_pcb* proceso_en_running;
t_list* lista_bloqueados_truncate;
t_list* lista_bloqueados_fread_fwrite;

// RECURSOS
t_list* recursos;

// SEMAFOROS
pthread_mutex_t mutex_new;
pthread_mutex_t mutex_ready;
pthread_mutex_t mutex_running;
sem_t maximo_grado_de_multiprogramacion;
sem_t cant_procesos_new;
sem_t cant_procesos_ready;
sem_t cpu_libre;
pthread_mutex_t mutex_pids;
sem_t fs_libre;

// SOCKETS
int server_cpu;
int server_fs;
int server_memoria;

// AUXILIARES
int pid_counter = 1;
t_list* lista_pids;
int cant_segmentos;


// LISTA GLOBAL DE PROCESOS
t_list* lista_global_procesos;

// TABLA GLOBAL DE ARCHIVO
t_list* tabla_global_archivos_abiertos;



// ------------------------------ MANEJO DE PROCESOS ------------------------------ //

void matar_proceso(char* motivo) {
    
    log_warning(logger, "Finaliza el proceso %d - Motivo: %s \n", proceso_en_running->pid, motivo); //LOG FIN DE PROCESO

    int socket_consola = proceso_en_running->socket_consola;
    int pid = proceso_en_running->pid;

    list_remove_element(lista_global_procesos, proceso_en_running); //add
    
    //mostrar_lista_global_procesos(lista_global_procesos);

    // Liberamos los recursos que tenia asignado el proceso
    // Iteramos la lista de recursos asignados del proceso, le sumamos 1 a todas las instancias y eliminamos el elemento de la lista
    liberar_recursos_asignados(proceso_en_running->recursos_asignados); 
    
    liberar_proceso(proceso_en_running);
    
    // Decirle a memoria que libere estructuras del proceso
    SEND_INT(server_memoria, SOLICITUD_LIBERAR_MEMORIA);
    SEND_INT(server_memoria, pid);
    
    // Avisarle a consola que finalizó el proceso.
    SEND_INT(socket_consola, pid);
    send_string(socket_consola, motivo);
        
    sem_post(&maximo_grado_de_multiprogramacion);
    sem_post(&cpu_libre);
}

void bloquear_proceso(args_io* argumentos_io){
    //sem_post(&cpu_libre); // A pesar de que el proceso se bloquee la CPU estará libre, así pueden seguir ejecutando otros procesos.
    
    int tiempo = argumentos_io->tiempo;
    t_pcb* proceso = argumentos_io->proceso;

    free(argumentos_io);

    log_warning(logger, "PID: %d - Ejecuta IO: %d \n", proceso->pid, tiempo);  //LOG I/O

    sleep(tiempo);

    log_warning(logger,"PID: %d - Estado anterior: BLOCKED - Estado actual: READY \n", proceso->pid); //LOG CAMBIO DE ESTADO


    mandar_a_ready(proceso);
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


t_pcb* buscar_proceso_por_pid_en_lista_global_procesos(t_list* lista ,int pid) {
    t_list_iterator* lista_it = list_iterator_create(lista);

    // si lo encuentra, lo saca de la lista y lo devuelve
    while (list_iterator_has_next(lista_it)) {
        t_pcb* proceso = (t_pcb*)list_iterator_next(lista_it);
        
        if (proceso->pid == pid) {
            list_iterator_destroy(lista_it);       
            return proceso;
        }
    }

    list_iterator_destroy(lista_it);
    return NULL;
}



void liberar_recursos_asignados(t_list* recursos_asignados) {
        
    t_list_iterator* lista_it = list_iterator_create(recursos_asignados);
                    
    while(list_iterator_has_next(lista_it)) {

        t_recurso* recurso = (t_recurso*)list_iterator_next(lista_it);

        recurso->cantidad_disponible++;
    }

    list_iterator_destroy(lista_it);   
}


void actualizar_tablas_segmentos(t_list* lista_recepcion_segmentos_actualizados) {
                        
    log_debug(logger, "Voy a actualizar todas mis tablas de procesos \n");
    
    // Tengo que iterar la tabla recibida, y por cada pid, buscar la tabla de procesos y ponerla en el pcb correspondiente    

    t_list_iterator* lista_it = list_iterator_create(lista_recepcion_segmentos_actualizados);

    while(list_iterator_has_next(lista_it)) {

        t_tabla_proceso* tabla_proceso = (t_tabla_proceso*)list_iterator_next(lista_it);

        t_pcb* proceso_a_modificar = buscar_proceso_por_pid_en_lista_global_procesos(lista_global_procesos, tabla_proceso->pid);

        list_destroy_and_destroy_elements(proceso_a_modificar->tabla_segmentos, free);

        proceso_a_modificar->tabla_segmentos = tabla_proceso->lista_segmentos;                        
    }

    list_iterator_destroy(lista_it);
    list_destroy_and_destroy_elements(lista_recepcion_segmentos_actualizados, free);
}






// ------------------------------ HRRN ------------------------------ //

void calcular_tasa_de_respuesta() {
    
    double tiempo_actual = (double)temporal_gettime(temporal);

    t_list_iterator* lista_it = list_iterator_create(procesos_en_ready);

    while(list_iterator_has_next(lista_it)) {
        t_pcb* proceso = (t_pcb*)list_iterator_next(lista_it);
        
        double tiempo_esperando_en_ready = tiempo_actual - proceso->tiempo_llegada_ready;
        
        proceso->tasa_de_respuesta = 1 + (tiempo_esperando_en_ready / proceso->estimacion_prox_rafaga);

        //log_debug(logger, "EL TIEMPO ESPERANDO EN READY DEL PROCESO %d es %f \n",proceso->pid, tiempo_esperando_en_ready);
        //log_debug(logger, "LA ESTIMACION DE RAFAGA DEL PROCESO %d es %f \n",proceso->pid, proceso->estimacion_prox_rafaga);
        //log_debug(logger, "LA TASA DE RESPUESTA DEL PROCESO %d es %f \n \n",proceso->pid, proceso->tasa_de_respuesta);
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
    t_pcb* proceso_tasa;  //No sabemos si hay que pedir memoria

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


// ------------------------------ MANEJO DE RECURSOS ------------------------------ //

t_recurso* recurso_en_lista(char* recurso_solicitado) {
    t_list_iterator* lista_it = list_iterator_create(recursos);

    while (list_iterator_has_next(lista_it)) {
        t_recurso* recurso = (t_recurso*)list_iterator_next(lista_it);
        
        if (string_equals_ignore_case(recurso->dispositivo, recurso_solicitado)) {
            list_iterator_destroy(lista_it);
            return recurso;
        }
    }
    
    list_iterator_destroy(lista_it);
    return NULL;
}


// ------------------------------ MANEJO DE ARCHIVOS ------------------------------ //

t_tabla_global_archivos_abiertos* buscar_archivo_en_tabla_global(char* archivo_solicitado) {
    t_list_iterator* lista_it = list_iterator_create(tabla_global_archivos_abiertos);

    while (list_iterator_has_next(lista_it)) {
        t_tabla_global_archivos_abiertos* archivo = (t_tabla_global_archivos_abiertos*)list_iterator_next(lista_it);
        
        if (string_equals_ignore_case(archivo->nombre, archivo_solicitado)) {
            list_iterator_destroy(lista_it);
            return archivo;
        }
    }
    
    list_iterator_destroy(lista_it);
    return NULL;
}

t_tabla_archivos_abiertos_proceso* buscar_archivo_en_tabla_archivos_por_proceso(t_pcb* proceso, char* archivo_solicitado) {
    t_list_iterator* lista_it = list_iterator_create(proceso->tabla_archivos_abiertos);

    while (list_iterator_has_next(lista_it)) {
        t_tabla_archivos_abiertos_proceso* archivo = (t_tabla_archivos_abiertos_proceso*)list_iterator_next(lista_it);
        
        if (string_equals_ignore_case(archivo->nombre, archivo_solicitado)) {
            list_iterator_destroy(lista_it);
            return archivo;
        }
    }
    
    list_iterator_destroy(lista_it);
    return NULL;
}


void mostrar_tabla_archivos_por_proceso(t_list* tabla){
    
    log_debug(logger, "Mostrando tabla de archivos abiertos por proceso: \n");

    t_list_iterator* lista_it = list_iterator_create(tabla);

    while (list_iterator_has_next(lista_it)) {
        t_tabla_archivos_abiertos_proceso* archivo = (t_tabla_archivos_abiertos_proceso*) list_iterator_next(lista_it);

        log_debug(logger, "Nombre archivo: %s - Puntero archivo: %d", archivo->nombre, archivo->puntero_archivo);

    }
    
    list_iterator_destroy(lista_it);
}


// ------------------------------ MANEJO DE LISTA BLOQUEADOS FREAD/FWRITE ------------------------------ //

void verificar_operaciones_terminadas(t_list* lista) {
    int valor_sem;
    sem_getvalue(&fs_libre, &valor_sem);
    if(list_is_empty(lista) && valor_sem == 0){
        sem_post(&fs_libre);
        log_info(logger, "Operaciones entre FS y Memoria finalizadas \n");
    }
}

