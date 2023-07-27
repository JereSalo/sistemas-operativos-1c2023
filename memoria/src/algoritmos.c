#include "algoritmos.h"

// La idea es retornar el hueco libre (dependiendo del algoritmo), pero no lo sacamos de la lista.
t_hueco* obtener_hueco_libre(int tamanio) {
    t_hueco* hueco_libre;
    
    switch(config_memoria.ALGORITMO_ASIGNACION) {

        case FIRST:
        {
            // Primer hueco libre de tabla de huecos cuyo tamanio sea mayor o igual a tamanio (suponiendo que esta ordenada por base)
            hueco_libre = obtener_primer_hueco(tamanio);
            break;
        }
        case BEST:
        {
            // Buscamos y obtenemos el hueco mas chico de la tabla de huecos cuyo tamaño sea mayor o igual a tamanio
            hueco_libre = obtener_hueco_mas_chico(tamanio);
            break;
        }
        case WORST:
        {
            // Buscamos y obtenemos el hueco mas grande de la tabla de huecos cuyo tamaño sea mayor o igual a tamanio
            hueco_libre = obtener_hueco_mas_grande(tamanio);
            break;
        }
    }

    return hueco_libre;
}


t_hueco* obtener_primer_hueco(int tamanio){
    t_list_iterator* lista_it = list_iterator_create(tabla_huecos);

    while(list_iterator_has_next(lista_it)) {
        t_hueco* hueco = (t_hueco*)list_iterator_next(lista_it);

        if(hueco->tamanio >= tamanio) {
            list_iterator_destroy(lista_it);
            return hueco;
        }
    }
    list_iterator_destroy(lista_it);
    return NULL;
}


t_hueco* obtener_hueco_mas_chico(int tamanio) {

    t_list_iterator* lista_it = list_iterator_create(tabla_huecos);

    int menor_tamanio = config_memoria.TAM_MEMORIA;
    t_hueco* hueco_mas_chico = NULL;

    while(list_iterator_has_next(lista_it)) {
        t_hueco* hueco = (t_hueco*)list_iterator_next(lista_it);

        if(hueco->tamanio <= menor_tamanio && hueco->tamanio >= tamanio) {
            menor_tamanio = hueco->tamanio;
            hueco_mas_chico = hueco;
        }
    }
    list_iterator_destroy(lista_it);

    return hueco_mas_chico;
}


t_hueco* obtener_hueco_mas_grande(int tamanio) {

    t_list_iterator* lista_it = list_iterator_create(tabla_huecos);

    int mayor_tamanio = 0;
    t_hueco* hueco_mas_grande = NULL;  

    while(list_iterator_has_next(lista_it)) {
        t_hueco* hueco = (t_hueco*)list_iterator_next(lista_it);

        if(hueco->tamanio > mayor_tamanio && hueco->tamanio >= tamanio) {
            mayor_tamanio = hueco->tamanio;
            hueco_mas_grande = hueco;
        }
    }
    list_iterator_destroy(lista_it);
    
    return hueco_mas_grande;
}


/*t_hueco* buscar_y_sacar_hueco(t_list* lista ,t_hueco* hueco_a_buscar) {
    t_list_iterator* lista_it = list_iterator_create(lista);

// si lo encuentra, lo saca de la lista y lo devuelve
    while (list_iterator_has_next(lista_it)) {
        t_hueco* hueco = (t_hueco*)list_iterator_next(lista_it);
        
        if (hueco->direccion_base == hueco_a_buscar->direccion_base) {
            list_iterator_destroy(lista_it);
            list_remove_element(lista, hueco);
           
            return proceso;
        }
    }
    
    list_iterator_destroy(lista_it);
    return NULL;
}*/
