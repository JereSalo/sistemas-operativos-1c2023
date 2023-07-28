#include "inicializar.h"

// ------------------------------ CONFIG KERNEL ------------------------------ //

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
    if(string_equals_ignore_case(string_algoritmo, "FIFO")){
        return FIFO;
    }
    if(string_equals_ignore_case(string_algoritmo, "HRRN")){
        return HRRN;
    }
    log_error(logger, "Hubo un error con el algoritmo de planificacion");
    return -1;
}


// ------------------------------ INICIALIZACION KERNEL ------------------------------ //

void inicializar_semaforos() {
    pthread_mutex_init(&mutex_new, NULL);
    pthread_mutex_init(&mutex_ready, NULL);
    pthread_mutex_init(&mutex_running, NULL);
    pthread_mutex_init(&mutex_pids, NULL);

    sem_init(&cant_procesos_new, 0, 0);
    sem_init(&cant_procesos_ready, 0, 0);
    sem_init(&maximo_grado_de_multiprogramacion, 0, config_kernel->GRADO_MAX_MULTIPROGRAMACION);

    sem_init(&cpu_libre, 0, 1);

    sem_init(&fs_libre, 0, 0);

    log_debug(logger, "Semaforos inicializados \n");
}

void inicializar_colas() {
    procesos_en_new = queue_create();
    procesos_en_ready = list_create();
    lista_pids = list_create();
    lista_bloqueados_truncate = list_create();
    lista_bloqueados_fread_fwrite = list_create();


    tabla_global_archivos_abiertos = list_create();
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


// ------------------------------ PCB ------------------------------ //

t_pcb* inicializar_pcb(int cliente_consola) {
    
    // Recibimos las instrucciones
    t_list* instrucciones_recibidas = list_create();

    if(!recv_instrucciones(cliente_consola, instrucciones_recibidas)){
        log_error(logger, "Fallo recibiendo INSTRUCCIONES \n");
    }

    // Recibimos la tabla de segmentos inicial
    t_list* tabla_segmentos = list_create();

    // send_opcode(server_memoria, SOLICITUD_TABLA_NEW);
    // SEND_INT(server_memoria, pid_counter);
    send_solicitud_tabla(server_memoria, pid_counter);

    if(!recv_tabla_segmentos(server_memoria, tabla_segmentos)){
        log_error(logger, "Fallo recibiendo TABLA DE SEGMENTOS \n");
    }

    // t_segmento* prueba = list_get(tabla_segmentos, 0);
    // log_info(logger, "ID SEGMENTO RECIBIDO: %d \n", prueba->id);
    // log_info(logger, "DIRECCION BASE SEGMENTO RECIBIDO: %d \n", prueba->direccion_base);
    // log_info(logger, "TAMANIO SEGMENTO RECIBIDO: %d \n", prueba->tamanio);


    // Creamos el PCB
    t_pcb* pcb = crear_pcb(pid_counter, instrucciones_recibidas, cliente_consola, tabla_segmentos);
    pid_counter++;
   
    return pcb;
}

t_pcb* crear_pcb(int pid, t_list* lista_instrucciones, int cliente_consola, t_list* tabla_segmentos) {
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->pid = pid;
    pcb->pc = 0;
    pcb->instrucciones = lista_instrucciones;
    pcb->registros_cpu = malloc(sizeof(t_registros_cpu));
    inicializar_registros(pcb->registros_cpu);
    pcb->recursos_asignados = list_create();
    
    pcb->tabla_segmentos = tabla_segmentos;   
    
    pcb->estimacion_prox_rafaga = config_kernel->ESTIMACION_INICIAL;            
    pcb->tiempo_llegada_ready = 0;
    pcb->tiempo_salida_running = 0;
    pcb->tiempo_llegada_running = 0;
    pcb->tasa_de_respuesta = 0;
    
    pcb->tabla_archivos_abiertos = list_create();
    pcb->socket_consola = cliente_consola;

    return pcb;
}