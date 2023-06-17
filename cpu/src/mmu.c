#include "mmu.h"

t_segmento* buscar_segmento(t_contexto_ejecucion* proceso, int num_segmento) {

    t_list_iterator* lista_it = list_iterator_create(proceso->tabla_segmentos);

    while (list_iterator_has_next(lista_it)) {
        t_segmento* segmento = (t_segmento*)list_iterator_next(lista_it);
        
        if (segmento->id_segmento == num_segmento) {
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

    t_segmento* segmento = buscar_segmento(proceso, num_segmento);

    if(desplazamiento_segmento + longitud_registro > segmento->tamanio_segmento){
        log_error(logger, "%d - Error SEG_FAULT- Segmento: %d - Offset: %d - Tamanio: %d", proceso->pid, num_segmento, desplazamiento_segmento, segmento->tamanio_segmento);
        return -1;
    }

    int direccion_fisica = segmento->direccion_base_segmento + desplazamiento_segmento;

    return direccion_fisica;
}