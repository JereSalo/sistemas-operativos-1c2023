#include "inicializar_memoria.h"

void inicializar_estructuras_administrativas() {
    
    
    memoria_principal = malloc(config_memoria.TAM_MEMORIA);
    tabla_huecos = list_create();
    tabla_segmentos_global = list_create();
    
    // Se crea hueco del tamaño de toda la memoria, se agrega a tabla de huecos
    crear_y_agregar_hueco(0, config_memoria.TAM_MEMORIA);
    
    // Creamos el segmento 0 -> no buscamos huecos libres porque ya sabemos que hay
    if(config_memoria.TAM_SEGMENTO_0 <= tamanio_max_segmento_cpu) {
        segmento_cero = crear_segmento(0, 0, config_memoria.TAM_SEGMENTO_0);
    }
    else
        log_error(logger, "No se puede crear el segmento porque excede el tamanio maximo permitido por CPU \n");


    // Actualizamos la tabla de huecos libres ya que hemos creado un segmento
    // De una forma similar haremos esto cuando creemos segmentos con CREATE_SEGMENT
    
    // t_hueco* hueco_actual = list_get(tabla_huecos, 0);

    // log_info(logger, "TAMANIO ACTUAL HUECO: %d \n", hueco_actual->tamanio_hueco);

    // hueco_actual->direccion_base_hueco += segmento_cero->tamanio_segmento;
    // hueco_actual->tamanio_hueco -= segmento_cero->tamanio_segmento;
    
    //t_hueco* hueco_nuevo = list_get(tabla_huecos, 0); //esto es para ver si se modifica correctamente
    
    //log_info(logger, "TAMANIO ACTUAL HUECO: %d \n", hueco_nuevo->tamanio_hueco);

    // Si yo modifico el elemento que retorna list_get se modifica tambien lo que esta en la lista
    

    log_info(logger, "Estructuras administrativas inicializadas \n");
}