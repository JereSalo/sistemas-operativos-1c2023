#include "inicializar.h"

void cargar_config_kernel(t_config* config) {
    
    config_kernel = malloc(sizeof(t_kernel_config));

    // IPs Y PUERTOS
    config_kernel->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    config_kernel->PUERTO_MEMORIA = config_get_int_value(config, "PUERTO_MEMORIA");

    config_kernel->IP_FILESYSTEM = config_get_string_value(config, "IP_FILESYSTEM");
    config_kernel->PUERTO_FILESYSTEM = config_get_int_value(config, "PUERTO_FILESYSTEM");

    config_kernel->IP_CPU = config_get_string_value(config, "IP_CPU");
    config_kernel->PUERTO_CPU = config_get_int_value(config, "PUERTO_CPU");

    config_kernel->PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");

    // PLANIFICACION
    config_kernel->ALGORITMO_PLANIFICACION = obtener_algoritmo_planificacion(config_get_string_value(config, "ALGORITMO_PLANIFICACION"));
    config_kernel->ESTIMACION_INICIAL = config_get_int_value(config, "ESTIMACION_INICIAL");
    config_kernel->HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");
    config_kernel->GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");

    // RECURSOS
    config_kernel->RECURSOS = config_get_array_value(config, "RECURSOS");
    config_kernel->INSTANCIAS_RECURSOS = config_get_array_value(config, "INSTANCIAS_RECURSOS");
    
    //log_info(logger, "Config cargada en config_kernel \n");
}

t_algoritmo_planificacion obtener_algoritmo_planificacion(char* string_algoritmo){
    if(strcmp(string_algoritmo, "FIFO") == 0){
        return FIFO;
    }
    if(strcmp(string_algoritmo, "HRRN") == 0){
        return HRRN;
    }
    log_error(logger, "Hubo un error con el algoritmo de planificacion");
    return -1;
}

void inicializar_semaforos() {
    pthread_mutex_init(&mutex_new, NULL);
    pthread_mutex_init(&mutex_ready, NULL);
    pthread_mutex_init(&mutex_running, NULL);
    pthread_mutex_init(&mutex_pids, NULL);

    //sem_init(&sem_proceso_en_running, 0, 0);
    sem_init(&cant_procesos_new, 0, 0);
    sem_init(&cant_procesos_ready, 0, 0);
    sem_init(&maximo_grado_de_multiprogramacion, 0, config_kernel->GRADO_MAX_MULTIPROGRAMACION);

    sem_init(&cpu_libre, 0, 1);

    //log_info(logger, "Semaforos inicializados \n");
}

void inicializar_colas() {
    procesos_en_new = queue_create();
    procesos_en_ready = list_create();
    lista_pids = list_create();
}

void inicializar_registros(t_registros_cpu* registros) {        
    asignar_a_registro("AX", "0000", registros);
    asignar_a_registro("BX", "0000", registros);
    asignar_a_registro("CX", "0000", registros);
    asignar_a_registro("DX", "0000", registros);

    asignar_a_registro("EAX", "00000000", registros);
    asignar_a_registro("EBX", "00000000", registros);
    asignar_a_registro("ECX", "00000000", registros);
    asignar_a_registro("EDX", "00000000", registros);
    
    asignar_a_registro("RAX", "0000000000000000", registros);
    asignar_a_registro("RBX", "0000000000000000", registros);
    asignar_a_registro("RCX", "0000000000000000", registros);
    asignar_a_registro("RDX", "0000000000000000", registros);
    
}


void inicializar_recursos() {
    recursos = list_create();    
    t_recurso* recurso;
    
    int i = 0;
    while(config_kernel->RECURSOS[i] != NULL) {
        recurso = malloc(sizeof(t_recurso));
        
        recurso->dispositivo = config_kernel->RECURSOS[i];
        recurso->cantidad_disponible = atoi(config_kernel->INSTANCIAS_RECURSOS[i]);
        recurso->cola_bloqueados = queue_create();

        list_add(recursos, recurso);

        i++;
    }
}


t_pcb* inicializar_pcb(int cliente_consola, int server_memoria) {
    
    // Recibimos las instrucciones
    t_list* instrucciones_recibidas = list_create();

    if(!recv_instrucciones(cliente_consola, instrucciones_recibidas)){
        log_error(logger, "Fallo recibiendo INSTRUCCIONES \n");
    }

    // Recibimos la tabla de segmentos inicial
    t_list* tabla_segmentos = list_create();

    // Solicitamos los segmentos a memoria mandandole un entero
    SEND_INT(server_memoria, 1);

    //if(!recv_segmentos(server_memoria, tabla_segmentos)){
    //    log_error(logger, "Fallo recibiendo TABLA DE SEGMENTOS \n");
    //}



    // Creamos el PCB
    t_pcb* pcb = crear_pcb(pid_counter, instrucciones_recibidas, cliente_consola);
    pid_counter++;
   
    return pcb;
}

t_pcb* crear_pcb(int pid, t_list* lista_instrucciones, int cliente_consola) {
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->pid = pid;
    pcb->pc = 0;
    pcb->instrucciones = lista_instrucciones;
    pcb->registros_cpu = malloc(sizeof(t_registros_cpu));
    inicializar_registros(pcb->registros_cpu);
    pcb->tabla_segmentos = list_create();                               //TODO: la dejamos como vacia pero la tabla la va a armar la memoria
    
    pcb->estimacion_prox_rafaga = config_kernel->ESTIMACION_INICIAL;            
    pcb->tiempo_llegada_ready = 0;
    pcb->tiempo_salida_running = 0;
    pcb->tiempo_llegada_running = 0;
    pcb->tasa_de_respuesta = 0;
    
    pcb->tabla_archivos_abiertos = list_create();
    pcb->socket_consola = cliente_consola;

    return pcb;
}