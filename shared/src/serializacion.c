#include "serializacion.h"


// ------------------------------ SERIALIZACION INSTRUCCIONES ------------------------------ //

void* serializar_lista_instrucciones(size_t* size, t_list* lista_instrucciones) {

    tamanio_lista(size, lista_instrucciones);
    
    void* stream = malloc(*size);

    //serializo elemento x elemento

    int desp = 0;

    t_list_iterator* lista_it = list_iterator_create(lista_instrucciones);

    char* item = list_iterator_next(lista_it);

    printf("VOY A SERIALIZAR EL MENSAJE %s DE TAMANIO %ld", item, strlen(item));

    memcpy(stream + desp, &item, sizeof(item));



    /*for (int i=0; list_iterator_has_next(lista_it); i++) {
        char* item = list_iterator_next(lista_it);

        printf("VOY A SERIALIZAR EL MENSAJE %s DE TAMANIO %ld", item, sizeof(item));

        memcpy(stream + desp, &item, sizeof(item));
        desp+=sizeof(item);
    }*/
    list_iterator_destroy(lista_it);
    return stream;
}


void* serializar_instrucciones(size_t* size, t_list* instrucciones) {
       
    // stream lista de instrucciones

    size_t size_instrucciones;
    
    void* stream_instrucciones = serializar_lista_instrucciones(&size_instrucciones, instrucciones);

    
    // stream completo

    //size_t size_total =  sizeof(op_code) 
     //                   + sizeof(size_t) // size total del stream
     //                   + sizeof(size_t) // size instrucciones
     //                   + size_instrucciones;   //instrucciones
    
   
   //void* stream = malloc(size_total);

    //op_code cop = INSTRUCCIONES;

   
   /* size_t size_payload = *size - sizeof(op_code) - sizeof(size_t);

    void* stream = malloc(*size);

    op_code cop = INSTRUCCIONES;
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &size_instrucciones, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t)*2 , instrucciones, size_instrucciones);
      
      */
    //return stream; 

    //free(stream_instrucciones);
    //*size = size_total;
    //return stream;       
}


//ESTAS DOS NO VAN EN ESTE .H

void tamanio_lista(size_t* size, t_list* lista) {

    t_list_iterator* lista_it = list_iterator_create(lista);
    for(int i = 0; list_iterator_has_next(lista_it); i++) {
        *size += (strlen(list_iterator_next(lista_it)));    //puede ser que tengamos que hacer +1 para el barra 0 ?
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










