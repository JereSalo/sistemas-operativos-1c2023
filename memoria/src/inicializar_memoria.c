#include "inicializar_memoria.h"

void inicializar_estructuras_administrativas() {
    memoria_principal = malloc(config_memoria.TAM_MEMORIA);
    tabla_huecos = list_create();
    lista_global_segmentos = list_create();
    tabla_segmentos_por_proceso = list_create();
    
    // Se crea hueco del tamaño de toda la memoria, se agrega a tabla de huecos
    crear_y_agregar_hueco(0, config_memoria.TAM_MEMORIA);
    
    // Creamos el segmento 0 -> no buscamos huecos libres porque ya sabemos que hay
    if(config_memoria.TAM_SEGMENTO_0 <= tamanio_max_segmento_cpu) {
        crear_segmento(0, 0, config_memoria.TAM_SEGMENTO_0);
        t_hueco* hueco = list_get(tabla_huecos, 0);
        hueco->direccion_base += config_memoria.TAM_SEGMENTO_0;
        hueco->tamanio -= config_memoria.TAM_SEGMENTO_0;
    }
    else{
        log_error(logger, "Segmento 0 excede size maximo permitido por CPU (Seg0: %d SizeMax: %d) \n", config_memoria.TAM_SEGMENTO_0, tamanio_max_segmento_cpu);
        exit(2);
    }
        
    segmento_cero = list_get(lista_global_segmentos, 0);

    log_info(logger, "Estructuras administrativas inicializadas \n");
}