#include "shared_utils.h"

// Este archivo está compuesto por funciones auxiliares, para que no ocupen espacio en otros archivos más específicos.

void* sumarSizeConLongitudString(void* a, void* b){
    
    size_t y = strlen((char*)b) + 1;
    *(size_t*)a += y;
    return a;
}

size_t tamanio_lista(t_list* lista){
    
    size_t* sumatoria = malloc(sizeof(size_t)); *sumatoria = 0;
    list_fold(lista, sumatoria, sumarSizeConLongitudString); // Devuelve lo foldeado pero hice que modificara sumatoria y listo, que al principio es la seed.
    size_t tamanio_lista = *sumatoria;
    free(sumatoria);
    return tamanio_lista;
}


void mostrar_lista(t_list* lista) {
    
    printf("mostrando lista\n");
    t_list_iterator* lista_it = list_iterator_create(lista);
    for(int i = 0; list_iterator_has_next(lista_it); i++) {
        printf("%s \n", (char*)list_iterator_next(lista_it));
    }
    list_iterator_destroy(lista_it);
}

void copiar_stream_en_variable_y_desplazar(void* variable, void* stream, size_t tamanio_elemento, size_t* desplazamiento){
    
    memcpy(variable, stream + *desplazamiento, tamanio_elemento);
    *desplazamiento += tamanio_elemento;
}


void copiar_variable_en_stream_y_desplazar(void* paquete, void* elemento, size_t tamanio_elemento, size_t* desplazamiento){
    
    memcpy(paquete + *desplazamiento, elemento, tamanio_elemento);
    *desplazamiento += tamanio_elemento;
}


void copiar_en_stream_y_desplazar_lista_strings_con_tamanios(void* paquete, t_list* lista_instrucciones){
    
    // El vscode se enoja con esto pero funca igual.
    // La definí como función auxiliar para poder usar al paquete
    // Me gustaria mandar el paquete como parámetro pero list_iterate admite funciones con 1 solo parámetro.
    size_t desplazamiento = 0;
    
    void copiar_y_desplazar_string_con_tamanio(void* string) {
        size_t tamanio = strlen((char*)string) + 1;
        copiar_variable_en_stream_y_desplazar(paquete, &tamanio, sizeof(size_t), &desplazamiento);
        copiar_variable_en_stream_y_desplazar(paquete, string, tamanio, &desplazamiento);
    }

    list_iterate(lista_instrucciones, copiar_y_desplazar_string_con_tamanio);
}