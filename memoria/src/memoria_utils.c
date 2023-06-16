#include "memoria_utils.h"

t_log* logger;
t_config *config;
t_memoria_config config_memoria;
int cliente_kernel;
int cliente_cpu;
int cliente_filesystem;

t_list* tabla_segmentos_global;
t_list* tabla_huecos;
void* memoria_principal;
t_segmento* segmento_cero;


void cargar_config_memoria(t_config* config){
    config_memoria.PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
    config_memoria.TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
    config_memoria.TAM_SEGMENTO_0 = config_get_int_value(config, "TAM_SEGMENTO_0");
    config_memoria.CANT_SEGMENTOS = config_get_int_value(config, "CANT_SEGMENTOS");
    config_memoria.RETARDO_MEMORIA = config_get_int_value(config, "RETARDO_MEMORIA");
    config_memoria.RETARDO_COMPACTACION = config_get_int_value(config, "RETARDO_COMPACTACION");
    config_memoria.ALGORITMO_ASIGNACION = obtener_algoritmo_asignacion(config_get_string_value(config, "ALGORITMO_ASIGNACION"));
}

void inicializar_estructuras() {
    
    // Inicializamos memoria principal
    memoria_principal = malloc(config_memoria.TAM_MEMORIA);

    // Inicializamos el segmento 0
    segmento_cero = malloc(sizeof(t_segmento));
    segmento_cero->id_segmento = 0;
    segmento_cero->direccion_base_segmento = 0; 
    segmento_cero->tamanio_segmento = config_memoria.TAM_SEGMENTO_0;

    // Inicializamos la tabla de segmentos global
    tabla_segmentos_global = list_create();

    // Inicializamos la tabla de huecos
    tabla_huecos = list_create();

    log_info(logger, "Estructuras administrativas inicializadas \n");
}





t_algoritmo_asignacion obtener_algoritmo_asignacion(char* string_algoritmo){
    if(strcmp(string_algoritmo, "FIRST") == 0){
        return FIRST;
    }
    if(strcmp(string_algoritmo, "BEST") == 0){
        return BEST;
    }
    if(strcmp(string_algoritmo, "WORST") == 0){
        return WORST;
    }
    log_error(logger, "Hubo un error con el algoritmo de asignacion");
    return -1;
}


void responder_pedido(t_orden orden){
    switch(orden.cod_orden){
        case LECTURA:
            // hace un send con el valor que corresponde a dicha direccion de memoria
            break;
        case ESCRITURA:
            // escribir(direccion_de_memoria, valor);
            // hace un send con el mensaje "OK" o "ERROR" luego de poder escribir o no en esa direccion
            break;
    }
}