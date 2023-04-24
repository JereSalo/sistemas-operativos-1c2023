#include "serializacion.h"


// ------------------------------ SERIALIZACION INSTRUCCIONES ------------------------------ //

void* serializar_instrucciones(size_t* size, char** instrucciones) {
            
}

void tamanio_lista(size_t* size, t_list* lista) {

    t_list_iterator* lista_it = list_iterator_create(lista);
    for(int i = 0; list_iterator_has_next(lista_it); i++) {
        *size += (strlen(list_iterator_next(lista_it)))+1;
    }
    list_iterator_destroy(lista_it);
}

void mostrar_lista(t_list* lista) {

    t_list_iterator* lista_it = list_iterator_create(lista);
    for(int i = 0; list_iterator_has_next(lista_it); i++) {
        printf("%s \n", list_iterator_next(lista_it));
    }
    list_iterator_destroy(lista_it);
}


// ------------------------------ SERIALIZACION NUMERO ------------------------------ //

void* serializar_numero(int numero) {
    
    //creamos un stream intermedio que guarde el codigo de operacion y el mensaje
    void* stream = malloc(sizeof(op_code) + sizeof(int));
    
    op_code codigo = NUMERO;

    //copiamos el codigo de operacion en el stream y despues nos corremos y copiamos el mensaje (payload)
    memcpy(stream, &codigo, sizeof(op_code));
    memcpy(stream+sizeof(op_code), &numero, sizeof(numero));
    
    // Aca retorna un stream con el codigo de operación al principio y concatendo a esto está nuestro dato (un int en este caso).
    return stream;
}


void deserializar_numero(void* stream, int* numero) {
    
    //aca estamos copiando el stream en la variable numero -> se recibe el mensaje
    memcpy(numero, stream, sizeof(int));                
}










