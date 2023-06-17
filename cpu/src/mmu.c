#include "mmu.h"

int traducir_direccion(int direccion_logica, t_contexto_ejecucion* proceso){
    
    int direccion_fisica;
    
    int tam_max_segmento = config_cpu.TAM_MAX_SEGMENTO;

    int num_segmento = floor(direccion_logica / tam_max_segmento);
    int desplazamiento_segmento = direccion_logica % tam_max_segmento;

    // Tenemos que buscar el segmento en la tabla de segmentos del proceso
    t_segmento* segmento = buscar_segmento(proceso, num_segmento);

    // Vemos cual es su direccion base
    direccion_fisica = segmento->direccion_base_segmento + desplazamiento_segmento;

    // A esa direccion base le sumamos el desplazamiento y asi obtenemos la direccion fisica

    return direccion_fisica;
}

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