#include "algoritmos.h"

// La idea es retornar el hueco libre (dependiendo del algoritmo), pero no lo sacamos de la lista.
t_hueco* buscar_hueco_libre() {

    t_hueco* hueco_libre;
    
    switch(config_memoria.ALGORITMO_ASIGNACION) {

        case FIRST:
        {
            // Primer hueco libre de tabla de huecos (suponiendo que esta ordenada por base)
            hueco_libre = list_get(tabla_huecos, 0);
            break;
        }
        case BEST:
        {
            // Buscamos y obtenemos el hueco mas chico de la tabla de huecos
            hueco_libre = buscar_hueco_mas_chico();
            break;
        }
        case WORST:
        {
            // Buscamos y obtenemos el hueco mas grande de la tabla de huecos
            hueco_libre = buscar_hueco_mas_grande();
            break;
        }
    }

    return hueco_libre;
}


t_hueco* buscar_hueco_mas_chico() {

    t_list_iterator* lista_it = list_iterator_create(tabla_huecos);

    int menor_tamanio = 0;
    t_hueco* hueco_mas_chico;  

    while(list_iterator_has_next(lista_it)) {
        t_hueco* hueco = (t_hueco*)list_iterator_next(lista_it);

        if(hueco->tamanio_hueco <= menor_tamanio) {
            menor_tamanio = hueco->tamanio_hueco;
            hueco_mas_chico = hueco;
        }
    }
    list_iterator_destroy(lista_it);

    // Removemos al hueco de la tabla y lo retornamos -> creo que no hay que removerlo
    //list_remove_element(tabla_huecos, hueco_mas_chico);
    return hueco_mas_chico;
}

t_hueco* buscar_hueco_mas_grande() {

    t_list_iterator* lista_it = list_iterator_create(tabla_huecos);

    int mayor_tamanio = 0;
    t_hueco* hueco_mas_grande;  

    while(list_iterator_has_next(lista_it)) {
        t_hueco* hueco = (t_hueco*)list_iterator_next(lista_it);

        if(hueco->tamanio_hueco > mayor_tamanio) {
            mayor_tamanio = hueco->tamanio_hueco;
            hueco_mas_grande = hueco;
        }
    }
    list_iterator_destroy(lista_it);
    
    // Removemos al hueco de la tabla y lo retornamos -> creo que no hay que removerlo
    //list_remove_element(tabla_huecos, hueco_mas_grande);
    return hueco_mas_grande;
}


/*t_hueco* buscar_y_sacar_hueco(t_list* lista ,t_hueco* hueco_a_buscar) {
    t_list_iterator* lista_it = list_iterator_create(lista);

// si lo encuentra, lo saca de la lista y lo devuelve
    while (list_iterator_has_next(lista_it)) {
        t_hueco* hueco = (t_hueco*)list_iterator_next(lista_it);
        
        if (hueco->direccion_base_hueco == hueco_a_buscar->direccion_base_hueco) {
            list_iterator_destroy(lista_it);
            list_remove_element(lista, hueco);
           
            return proceso;
        }
    }
    
    list_iterator_destroy(lista_it);
    return NULL;
}*/
