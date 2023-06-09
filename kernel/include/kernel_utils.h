#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include "shared.h"
// Todos los archivos que esten en la carpeta kernel van a tener include de kernel_utils.h

extern t_log* logger;



extern t_temporal* temporal;
extern double tiempo;



typedef struct {
    
    char* IP_KERNEL;
	char* IP_MEMORIA;
    int PUERTO_MEMORIA;
    char* IP_FILESYSTEM;
    int PUERTO_FILESYSTEM;
    char* IP_CPU;
    int PUERTO_CPU;
    int PUERTO_ESCUCHA;
    char* ALGORITMO_PLANIFICACION;
    int ESTIMACION_INICIAL;
    double HRRN_ALFA;                   // Es double por el config_get_double_value
    int GRADO_MAX_MULTIPROGRAMACION;
    char** RECURSOS;
    char** INSTANCIAS_RECURSOS;

} t_kernel_config;

extern t_kernel_config* config_kernel;


typedef struct {
    int pid;                            // process id: identificador del proceso.
    int pc;                             // program counter: número de la próxima instrucción a ejecutar.
    t_registros_cpu* registros_cpu;
    t_list* instrucciones;              // lista de instrucciones a ejecutar. t_list*
	t_list* tabla_segmentos;            // va a contener elementos de tipo t_segmento
    double estimacion_prox_rafaga;      // Para HRRN
    double tiempo_llegada_ready;        // Para HRRN
    double tiempo_llegada_running;      // Para HRRN
    double tiempo_salida_running;       // Para HRRN
    double tasa_de_respuesta;           // Para HRRN
    t_list* tabla_archivos_abiertos;    // va a contener elementos de tipo FILE*
    int socket_consola;
} t_pcb;


typedef struct {
    char* dispositivo;
    int cantidad_disponible;
    t_queue* cola_bloqueados; 
} t_recurso;

typedef struct{
    t_pcb* proceso;
    int tiempo;
}args_io;


// VARIABLES PARA PCB

extern int pid_counter;

// Estados de procesos
extern t_queue* procesos_en_new;
extern t_list* procesos_en_ready;
extern t_pcb* proceso_en_running;

extern t_list* recursos;


extern t_list* lista_pids;

// Semáforos
extern pthread_mutex_t mutex_new;
extern pthread_mutex_t mutex_ready;
extern pthread_mutex_t mutex_running;
extern pthread_mutex_t mutex_pids;

extern sem_t maximo_grado_de_multiprogramacion;
extern sem_t cant_procesos_new;
extern sem_t cant_procesos_ready;

extern sem_t cpu_libre;

extern int server_cpu;


// FUNCIONES EN KERNEL_UTILS.C


void matar_proceso(char* motivo);

t_pcb* buscar_y_sacar_proceso(t_list* lista ,t_pcb* proceso_a_buscar);
void buscar_y_borrar_proceso(t_list* lista ,t_pcb* proceso_a_buscar);
t_recurso* recurso_en_lista(char* recurso_solicitado);
void bloquear_proceso(args_io* argumentos_io);


void cargar_contexto_de_ejecucion(t_pcb*, t_contexto_ejecucion*);

// HRRN
void estimar_proxima_rafaga(t_pcb* proceso);
void calcular_tasa_de_respuesta(double tiempo_actual);
t_pcb* proceso_con_mayor_tasa_de_respuesta() ;




// Funciones de otros modulos


void volver_a_encolar_en_ready(t_pcb* proceso);

void inicializar_registros(t_registros_cpu* registros);





#endif