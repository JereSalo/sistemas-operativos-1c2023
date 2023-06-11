#ifndef SHARED_UTILS_H_
#define SHARED_UTILS_H_

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
#include <commons/temporal.h>
#include <netinet/in.h>
#include <arpa/inet.h>




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
    int id;
    int direccion_base; // ???
	int size; // Tamaño de segmento
} t_segmento;

typedef enum {
    SUCCESS,
    SEG_FAULT,
    OUT_OF_MEMORY
} t_motivo_fin_proceso;

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
    int pid;  
    int pc; 
    t_registros_cpu* registros_cpu;
    t_list* instrucciones;
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
    EXIT
} op_instruccion;





extern t_dictionary* diccionario_instrucciones;
extern t_dictionary* diccionario_registros_cpu;

void mostrar_lista(t_list* lista);
void* sumarSizeConLongitudString(void* a, void* b);
size_t tamanio_lista(t_list* lista);

void copiar_variable_en_stream_y_desplazar(void* paquete, void* elemento, size_t tamanio_elemento, size_t* desplazamiento);
void copiar_en_stream_y_desplazar_lista_strings_con_tamanios(void* paquete, t_list* lista_instrucciones);
void copiar_stream_en_variable_y_desplazar(void* variable, void* stream, size_t tamanio_elemento, size_t* desplazamiento);
void asignar_a_registro(char* registro, char* valor, t_registros_cpu* registros);
void inicializar_diccionarios();
char* lista_a_string(t_list* lista, char string[]);
char* lista_pids_a_string(t_list* lista, char string[]);

t_contexto_ejecucion* crear_contexto();
void registros_add_all(t_registros_cpu* registros_destino, t_registros_cpu* registros_origen);
t_contexto_ejecucion* cargar_contexto(t_pcb* proceso);
void liberar_contexto(t_contexto_ejecucion* contexto);
void liberar_proceso(t_pcb* proceso);

#endif