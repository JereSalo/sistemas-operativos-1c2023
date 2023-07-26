#ifndef MEMORIA_UTILS_H
#define MEMORIA_UTILS_H

#include "shared.h"

// ------------------------------ STRUCTS / ENUMS MEMORIA ------------------------------ //

typedef enum {
    FIRST,
    BEST,
    WORST
} t_algoritmo_asignacion;

typedef struct {
    int PUERTO_ESCUCHA;
    int TAM_MEMORIA;
    int TAM_SEGMENTO_0;
    int CANT_SEGMENTOS;
    int RETARDO_MEMORIA;
    int RETARDO_COMPACTACION;
    t_algoritmo_asignacion ALGORITMO_ASIGNACION;
} t_memoria_config;


// ------------------------------ VARIABLES GLOBALES ------------------------------ //


extern pthread_mutex_t mutex_falopa;

// LOGGER Y CONFIG
extern t_log* logger;
extern t_config *config;
extern t_memoria_config config_memoria;

// SOCKETS
extern int cliente_kernel;
extern int cliente_cpu;
extern int cliente_filesystem;

// ESTRUCTURAS ADMINISTRATIVAS 
extern t_list* lista_global_segmentos;
extern t_list* tabla_segmentos_por_proceso;
extern t_list* tabla_huecos;
extern void* memoria_principal;
extern t_segmento* segmento_cero;
extern int tamanio_max_segmento_cpu;

// ------------------------------ CONFIG MEMORIA ------------------------------ //
void cargar_config_memoria(t_config* config);
t_algoritmo_asignacion obtener_algoritmo_asignacion(char* string_algoritmo);


// ------------------------------ MANEJO DE HUECOS ------------------------------ //
t_hueco* crear_hueco(int direccion_base, int tamanio);
void agregar_hueco(t_hueco* hueco);
void crear_y_agregar_hueco(int direccion_base, int tamanio);
t_hueco* consolidar_huecos(t_hueco* hueco_original, t_hueco* hueco_aledanio_1, t_hueco* hueco_aledanio_2);
void crear_y_consolidar_huecos(int direccion_base, int tamanio);
t_hueco* buscar_hueco_por_base(int direccion_base);
t_hueco* buscar_hueco_por_final(int direccion_final);


// ------------------------------ MANEJO DE SEGMENTOS ------------------------------ //
t_segmento* crear_segmento(int id, int direccion_base, int tamanio);
void agregar_segmento(t_segmento* segmento, int pid);
t_segmento* buscar_segmento_por_base(int id_segmento, t_list* tabla_segmentos);
t_tabla_proceso* buscar_proceso_por_pid(t_list* lista ,int pid);
t_segmento* buscar_segmento_en_tabla_por_proceso(t_list* tabla_segmentos, t_tabla_proceso** proceso, int pid, int id_segmento);
t_segmento* buscar_segmento_en_tabla_global(t_list* tabla_segmentos, int direccion_base);
void eliminar_segmento_de_tabla(t_list* tabla_segmentos, t_segmento* segmento, char* tipo_tabla, int pid);
t_segmento* mover_segmentos();
void mostrar_tabla_segmentos_proceso(t_list* tabla_segmentos, int pid);
void mostrar_resultado_compactacion(t_list* tabla_proceso);



// ------------------------------ OTROS ------------------------------ // 
int espacio_restante_memoria();
void leer_memoria();


#endif