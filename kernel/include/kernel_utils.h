#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include "shared.h"
// Todos los archivos que esten en la carpeta kernel van a tener include de kernel_utils.h

// ------------------------------ STRUCTS / ENUMS KERNEL ------------------------------ //

typedef enum {
    FIFO,
    HRRN
} t_algoritmo_planificacion;

typedef struct {
    
    char* IP_KERNEL;
	char* IP_MEMORIA;
    int PUERTO_MEMORIA;
    char* IP_FILESYSTEM;
    int PUERTO_FILESYSTEM;
    char* IP_CPU;
    int PUERTO_CPU;
    int PUERTO_ESCUCHA;
    t_algoritmo_planificacion ALGORITMO_PLANIFICACION;
    int ESTIMACION_INICIAL;
    double HRRN_ALFA;                   // Es double por el config_get_double_value
    int GRADO_MAX_MULTIPROGRAMACION;
    char** RECURSOS;
    char** INSTANCIAS_RECURSOS;

} t_kernel_config;

typedef struct {
    char* dispositivo;
    int cantidad_disponible;
    t_queue* cola_bloqueados; 
} t_recurso;

typedef struct{
    t_pcb* proceso;
    int tiempo;
}args_io;


// ------------------------------ VARIABLES GLOBALES ------------------------------ //

// LOGGER Y CONFIG
extern t_log* logger;
extern t_config* config;
extern t_kernel_config* config_kernel;

// SOCKETS
extern int server_cpu;
extern int server_fs;
extern int server_memoria;

// HRRN 
extern t_temporal* temporal;
extern double tiempo;

// COLAS DE ESTADOS DE PROCESOS
extern t_queue* procesos_en_new;
extern t_list* procesos_en_ready;
extern t_pcb* proceso_en_running;
extern t_list* lista_bloqueados_truncate;   // Aca van a ir los procesos que se bloqueen al hacer F_WRITE/READ/TRUNCATE
extern t_list* lista_bloqueados_fread_fwrite;


// RECURSOS
extern t_list* recursos;

// SEMAFOROS
extern pthread_mutex_t mutex_new;
extern pthread_mutex_t mutex_ready;
extern pthread_mutex_t mutex_running;
extern pthread_mutex_t mutex_pids;

extern sem_t maximo_grado_de_multiprogramacion;
extern sem_t cant_procesos_new;
extern sem_t cant_procesos_ready;

extern sem_t cpu_libre;

extern sem_t fs_libre;

// AUXILIARES
extern t_list* lista_pids;
extern int pid_counter;
extern int cant_segmentos;

// LISTA GLOBAL DE PROCESOS
extern t_list* lista_global_procesos;

// TABLA GLOBAL DE ARCHIVOS ABIERTOS
extern t_list* tabla_global_archivos_abiertos;


// ------------------------------ MANEJO DE PROCESOS ------------------------------ //
void matar_proceso(char* motivo);
void bloquear_proceso(args_io* argumentos_io);
t_pcb* buscar_y_sacar_proceso(t_list* lista ,t_pcb* proceso_a_buscar);
t_pcb* buscar_proceso_por_pid_en_lista_global_procesos(t_list* lista ,int pid);



// ------------------------------ HRRN ------------------------------ //
void calcular_tasa_de_respuesta();
void estimar_proxima_rafaga(t_pcb* proceso);
t_pcb* proceso_con_mayor_tasa_de_respuesta() ;


// ------------------------------ MANEJO DE RECURSOS ------------------------------ //
t_recurso* recurso_en_lista(char* recurso_solicitado);
void liberar_recursos_asignados(t_list* recursos_asignados);
void actualizar_tablas_segmentos(t_list* lista_recepcion_segmentos_actualizados);


// ------------------------------ MANEJO DE ARCHIVOS ------------------------------ //
t_tabla_global_archivos_abiertos* buscar_archivo_en_tabla_global(char* archivo_solicitado);
t_tabla_archivos_abiertos_proceso* buscar_archivo_en_tabla_archivos_por_proceso(t_pcb* proceso, char* archivo_solicitado);
void mostrar_tabla_archivos_por_proceso(t_list* tabla);

// Funciones de otros modulos
void mandar_a_ready(t_pcb* proceso);
void inicializar_registros(t_registros_cpu* registros);

void verificar_operaciones_terminadas(t_list* lista);

#endif