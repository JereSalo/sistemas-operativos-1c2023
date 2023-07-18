#ifndef SHARED_UTILS_H_
#define SHARED_UTILS_H_

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>
#include <pthread.h>
#include "commons/collections/queue.h"
#include <semaphore.h>
#include <stdint.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include <commons/temporal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <dirent.h>
#include <limits.h>



typedef struct{
    char AX[4], BX[4], CX[4], DX[4];
    char EAX[8], EBX[8], ECX[8], EDX[8];
    char RAX[16], RBX[16], RCX[16], RDX[16];
} t_registros_cpu;

typedef enum {
    AX, BX, CX, DX,
    EAX, EBX, ECX, EDX,
    RAX, RBX, RCX, RDX
} registro_cpu;


typedef struct {
    int pid;                            // process id: identificador del proceso.
    int pc;                             // program counter: número de la próxima instrucción a ejecutar.
    t_registros_cpu* registros_cpu;
    t_list* instrucciones;              // lista de instrucciones a ejecutar. t_list*
    t_list* recursos_asignados;         // esto sirve para despues cuando muera, liberar recursos
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
    char* nombre;
    int tamanio;
    uint32_t puntero_directo;     // apunta al primer bloque de datos del archivo
    uint32_t puntero_indirecto;   // apunta al primer bloque de datos del archivo
} t_fcb;


typedef struct {
    char* nombre;
    bool esta_abierto;
    t_queue* cola_bloqueados;
} t_tabla_global_archivos_abiertos;



typedef struct {
    char* nombre;
    uint32_t puntero_archivo;
} t_tabla_archivos_abiertos_proceso;



typedef struct {
    int id;
    int direccion_base;    
    int tamanio;
} t_segmento;

typedef struct {
    int direccion_base;
    int tamanio;
} t_hueco;

typedef struct {
	int pid;
	t_list* lista_segmentos;
} t_tabla_proceso;

typedef struct {
    int pid;  
    int pc; 
    t_registros_cpu* registros_cpu;
    t_list* instrucciones;
    t_list* tabla_segmentos;
} t_contexto_ejecucion;

typedef enum {
    SET,
    MOV_IN,
    MOV_OUT,
    IO,
    F_OPEN,
    F_CLOSE,
    F_SEEK,
    F_READ,
    F_WRITE,
    F_TRUNCATE,
    WAIT,
    SIGNAL,
    CREATE_SEGMENT,
    DELETE_SEGMENT,
    YIELD,
    EXIT,
    SEG_FAULT // No es una instruccion pero no me cabe una
} op_instruccion;

typedef enum {
    CREACION,
    COMPACTACION,
    OUT_OF_MEMORY
} op_respuesta_memoria;



extern t_dictionary* diccionario_instrucciones;
extern t_dictionary* diccionario_registros_cpu;

void mostrar_lista(t_list* lista);
void mostrar_tabla_huecos(t_list* tabla_huecos);
void mostrar_tabla_segmentos(t_list* tabla_segmentos);
void* sumarSizeConLongitudString(void* a, void* b);
size_t tamanio_lista(t_list* lista);

void copiar_variable_en_stream_y_desplazar(void* paquete, void* elemento, size_t tamanio_elemento, size_t* desplazamiento);
void copiar_en_stream_y_desplazar_lista_strings_con_tamanios(void* paquete, t_list* lista_instrucciones);
void copiar_en_stream_y_desplazar_tabla_segmentos(void* paquete, t_list* tabla_segmentos); //ADD
void copiar_stream_en_variable_y_desplazar(void* variable, void* stream, size_t tamanio_elemento, size_t* desplazamiento);
int obtener_longitud_registro(char* registro);
char* obtener_registro_objetivo(t_registros_cpu* registros, char* nombre_registro);
void asignar_a_registro(char* registro, char* valor, t_registros_cpu* registros);
char* leer_de_registro(char* registro, t_registros_cpu* registros);
void inicializar_diccionarios();
char* lista_a_string(t_list* lista, char string[]);
char* lista_pids_a_string(t_list* lista, char string[]);

t_contexto_ejecucion* crear_contexto();
void registros_copypaste(t_registros_cpu* registros_destino, t_registros_cpu* registros_origen);
t_contexto_ejecucion* cargar_contexto(t_pcb* proceso);
void liberar_contexto(t_contexto_ejecucion* contexto);
void liberar_proceso(t_pcb* proceso);
void lista_copypaste(t_list* lista_objetivo, t_list* lista_origen);
void tabla_copypaste(t_list* lista_objetivo, t_list* lista_origen);
t_segmento* duplicar_segmento(const t_segmento* original);

t_segmento* buscar_segmento_por_id(int id_segmento, t_list* tabla_segmentos);

void mostrar_lista_global_procesos(t_list* lista);


#endif