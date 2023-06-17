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


void cargar_config_memoria(t_config* config){
    config_memoria.PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
    config_memoria.TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
    config_memoria.TAM_SEGMENTO_0 = config_get_int_value(config, "TAM_SEGMENTO_0");
    config_memoria.CANT_SEGMENTOS = config_get_int_value(config, "CANT_SEGMENTOS");
    config_memoria.RETARDO_MEMORIA = config_get_int_value(config, "RETARDO_MEMORIA");
    config_memoria.RETARDO_COMPACTACION = config_get_int_value(config, "RETARDO_COMPACTACION");
    config_memoria.ALGORITMO_ASIGNACION = obtener_algoritmo_asignacion(config_get_string_value(config, "ALGORITMO_ASIGNACION"));
}

void inicializar_estructuras_administrativas() {
    
    // Inicializamos memoria principal
    memoria_principal = malloc(config_memoria.TAM_MEMORIA);

    // Inicializamos la tabla de huecos
    tabla_huecos = list_create();
    
    // Inicializamos la tabla de segmentos global
    tabla_segmentos_global = list_create();
    
    // Creamos el hueco inicial cuyo tamanio es toda la memoria principal y lo aniadimos a la tabla de huecos
    t_hueco* hueco = crear_hueco(0, config_memoria.TAM_MEMORIA);
    list_add(tabla_huecos, hueco);

    // Creamos el segmento 0 -> no buscamos huecos libres porque ya sabemos que hay
    if(config_memoria.TAM_SEGMENTO_0 <= tamanio_max_segmento_cpu) {
        segmento_cero = crear_segmento(0, 0, config_memoria.TAM_SEGMENTO_0);
    }
    else
        log_error(logger, "No se puede crear el segmento porque excede el tamanio maximo permitido por CPU \n");


    // Actualizamos la tabla de huecos libres ya que hemos creado un segmento
    // De una forma similar haremos esto cuando creemos segmentos con CREATE_SEGMENT
    
    t_hueco* hueco_actual = list_get(tabla_huecos, 0);

    log_info(logger, "TAMANIO ACTUAL HUECO: %d \n", hueco_actual->tamanio_hueco);

    hueco_actual->direccion_base_hueco += segmento_cero->tamanio_segmento;
    hueco_actual->tamanio_hueco -= segmento_cero->tamanio_segmento;
    
    //t_hueco* hueco_nuevo = list_get(tabla_huecos, 0); //esto es para ver si se modifica correctamente
    
    //log_info(logger, "TAMANIO ACTUAL HUECO: %d \n", hueco_nuevo->tamanio_hueco);

    // Si yo modifico el elemento que retorna list_get se modifica tambien lo que esta en la lista
    

    log_info(logger, "Estructuras administrativas inicializadas \n");
}

t_hueco* crear_hueco(int direccion_base, int tamanio) {

    t_hueco* hueco = malloc(sizeof(t_hueco));

    hueco->direccion_base_hueco = direccion_base;
    hueco->tamanio_hueco = tamanio;

    return hueco;
}


t_segmento* crear_segmento(int id, int direccion_base, int tamanio) {

    t_segmento* segmento = malloc(sizeof(t_segmento));

    segmento->id_segmento = id;
    segmento->direccion_base_segmento = direccion_base;
    segmento->tamanio_segmento = tamanio;

    return segmento;
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