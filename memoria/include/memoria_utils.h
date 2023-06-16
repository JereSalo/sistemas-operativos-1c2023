#ifndef MEMORIA_UTILS_H
#define MEMORIA_UTILS_H

#include "shared.h"

typedef struct {
    int id_segmento;
    int direccion_base_segmento;    
    int tamanio_segmento;
} t_segmento;


typedef struct {
	int pid;
	t_list* lista_segmentos;
} t_tabla_proceso;

typedef struct {
    int* direccion_base_hueco;
    int tamanio_hueco;
} t_hueco;

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

extern t_log* logger;
extern t_config *config;
extern t_memoria_config config_memoria;
extern int cliente_kernel;
extern int cliente_cpu;
extern int cliente_filesystem;

extern t_list* tabla_segmentos_global;
extern t_list* tabla_huecos;
extern void* memoria_principal;
extern t_segmento* segmento_cero;




void cargar_config_memoria(t_config* config);
void inicializar_estructuras();
t_algoritmo_asignacion obtener_algoritmo_asignacion(char* string_algoritmo);






// Esto podriamos moverlo despues a otro archivo que no sea el utils.h
typedef enum {
    ESCRITURA,
    LECTURA
} t_cod_orden;

typedef struct {
    t_cod_orden cod_orden;
    int direccion;
    int valor;
} t_orden;

void responder_pedido(t_orden orden);

#endif