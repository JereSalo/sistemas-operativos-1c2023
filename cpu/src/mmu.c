#include "mmu.h"

// ------------------------------ TRADUCCION DE DIRECCIONES ------------------------------ //

t_segmento* buscar_segmento(t_contexto_ejecucion* proceso, int num_segmento) {

    t_list_iterator* lista_it = list_iterator_create(proceso->tabla_segmentos);

    while (list_iterator_has_next(lista_it)) {
        t_segmento* segmento = (t_segmento*)list_iterator_next(lista_it);
        
        if (segmento->id == num_segmento) {
            list_iterator_destroy(lista_it);
           
            return segmento;
        }
    }
    
    list_iterator_destroy(lista_it);
    return NULL;
}

// Retorna la direccion si no hay seg fault, sino retorna -1
int obtener_direccion(int direccion_logica, t_contexto_ejecucion* proceso, char* nombre_registro){
    int tam_max_segmento = config_cpu.TAM_MAX_SEGMENTO;
    int longitud_registro = obtener_longitud_registro(nombre_registro);

    int num_segmento = floor(direccion_logica / tam_max_segmento);
    int desplazamiento_segmento = direccion_logica % tam_max_segmento;

    t_segmento* segmento = buscar_segmento(proceso, num_segmento); //busca por id

    //if(desplazamiento_segmento + longitud_registro > segmento->tamanio){
    //    log_error(logger, "%d - Error SEG_FAULT- Segmento: %d - Offset: %d - Tamanio: %d", proceso->pid, num_segmento, desplazamiento_segmento, segmento->tamanio);
    //    return -1;
    //}

    //int direccion_fisica = segmento->direccion_base + desplazamiento_segmento;

    int direccion_fisica = direccion_logica;

    log_debug(logger, "Direccion Fisica: %d", direccion_fisica);
    log_debug(logger, "%d - Segmento: %d - Offset: %d - Tamanio: %d", proceso->pid, num_segmento, desplazamiento_segmento, segmento->tamanio);

    return direccion_fisica;
}