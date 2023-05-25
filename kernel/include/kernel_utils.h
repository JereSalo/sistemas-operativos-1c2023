#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include "shared.h"
// Todos los archivos que esten en la carpeta kernel van a tener include de kernel_utils.h

extern t_log* logger;

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
    double HRRN_ALFA; // Es double por el config_get_double_value
    int GRADO_MAX_MULTIPROGRAMACION;
    //t_list* RECURSOS;
    //t_list* INSTANCIAS_RECURSOS;

} t_kernel_config;
extern t_kernel_config config_kernel;


typedef struct {
    int pid;                            // process id: identificador del proceso.
    //int size;
    int pc;                             // program counter: número de la próxima instrucción a ejecutar.
	//int cliente_socket;
    t_registros_cpu* registros_cpu;
    t_list* instrucciones;              // lista de instrucciones a ejecutar. t_list*
	t_list* tabla_segmentos;            // va a contener elementos de tipo t_segmento
    double estimacion_prox_rafaga;      // Para HRRN
    double tiempo_llegada_ready;        // Para HRRN
    t_list* tabla_archivos_abiertos;    // va a contener elementos de tipo FILE*
} t_pcb;

// VARIABLES PARA PCB

extern int pid_counter;

// Estados de procesos
extern t_queue* procesos_en_new;
extern t_list* procesos_en_ready;
extern t_pcb* proceso_en_running;


// Semáforos
extern pthread_mutex_t mutex_new;
extern pthread_mutex_t mutex_ready;
extern pthread_mutex_t mutex_running;

extern sem_t maximo_grado_de_multiprogramacion;
extern sem_t cant_procesos_new;
extern sem_t cant_procesos_ready;



// FUNCIONES EN KERNEL_UTILS.C

void cargar_config_kernel(t_config* config);

void inicializar_semaforos();
void inicializar_colas();
t_pcb* inicializar_pcb(int cliente_socket);
t_pcb* crear_pcb(int pid, t_list* lista_instrucciones);

void esperar_clientes_kernel(int server_socket);
void procesar_conexion_kernel(void* cliente_socket);

void inicializar_registros(t_registros_cpu* registros);

//void contexto_de_ejecucion(t_pcb* proceso, t_contexto_ejecucion* contexto_ejecucion);

void procesar_conexion_kernel_cpu(void* void_cliente_socket);


#endif