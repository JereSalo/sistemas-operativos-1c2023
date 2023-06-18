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
int tamanio_max_segmento_cpu;

// CONFIG

void cargar_config_memoria(t_config* config){
    config_memoria.PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
    config_memoria.TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
    config_memoria.TAM_SEGMENTO_0 = config_get_int_value(config, "TAM_SEGMENTO_0");
    config_memoria.CANT_SEGMENTOS = config_get_int_value(config, "CANT_SEGMENTOS");
    config_memoria.RETARDO_MEMORIA = config_get_int_value(config, "RETARDO_MEMORIA");
    config_memoria.RETARDO_COMPACTACION = config_get_int_value(config, "RETARDO_COMPACTACION");
    config_memoria.ALGORITMO_ASIGNACION = obtener_algoritmo_asignacion(config_get_string_value(config, "ALGORITMO_ASIGNACION"));
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


t_hueco* crear_hueco(int direccion_base, int tamanio){
    t_hueco* hueco = malloc(sizeof(t_hueco));

    hueco->direccion_base_hueco = direccion_base;
    hueco->tamanio_hueco = tamanio;

    return hueco;
}

// Crea hueco y lo agrega a la tabla de huecos
void agregar_hueco(t_hueco* hueco){
    list_add(tabla_huecos, hueco);
}

void crear_y_agregar_hueco(int direccion_base, int tamanio){
    t_hueco* hueco = crear_hueco(direccion_base, tamanio);
    agregar_hueco(hueco);
}


// Crea segmento, actualizando tabla de huecos
//TODO
t_segmento* crear_segmento(int id, int direccion_base, int tamanio) {
    t_segmento* segmento = malloc(sizeof(t_segmento));

    segmento->id_segmento = id;
    segmento->direccion_base_segmento = direccion_base;
    segmento->tamanio_segmento = tamanio;

    // Actualizar tabla de huecos.

    t_hueco* hueco = buscar_hueco_por_base(direccion_base);
    hueco->direccion_base_hueco += segmento->tamanio_segmento;
    hueco->tamanio_hueco -= segmento->tamanio_segmento;

    if(hueco->tamanio_hueco == 0){
        list_remove_element(tabla_huecos, hueco);
        free(hueco);
    }
}

t_hueco* buscar_hueco_por_base(int direccion_base){
    bool coincide_con_base(t_hueco* hueco){
        hueco->direccion_base_hueco == direccion_base;
    }

    list_find(tabla_huecos, coincide_con_base);
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