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

void bloquear_proceso(args_io* argumentos_io);


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

extern int cliente_socket_cpu;


// FUNCIONES EN KERNEL_UTILS.C

void cargar_config_kernel(t_config* config);

void inicializar_semaforos();
void inicializar_colas();

t_pcb* inicializar_pcb(int cliente_socket);
t_pcb* crear_pcb(int pid, t_list* lista_instrucciones, int cliente_socket);

void esperar_clientes_kernel(int server_socket);

void inicializar_registros(t_registros_cpu* registros);

void procesar_consola(void* cliente_socket);
void procesar_cpu(void* void_cliente_socket);

void manejar_proceso_desalojado(op_instruccion motivo_desalojo, t_list* lista_parametros);
void matar_proceso(char* motivo); // Prototipo de la función pero no está definida en kernel_utils
void volver_a_encolar_en_ready(t_pcb* proceso);

void inicializar_recursos();

t_recurso* recurso_en_lista(char* recurso_solicitado);

void volver_a_running();

t_pcb* buscar_y_sacar_proceso(t_list* lista ,t_pcb* proceso_a_buscar);
void buscar_y_borrar_proceso(t_list* lista ,t_pcb* proceso_a_buscar);

#endif