#include "serializacion.h"


void copiar_y_desplazar(void* paquete, void* elemento, size_t tamanio_elemento, size_t* desplazamiento){
    memcpy(paquete + *desplazamiento, elemento, tamanio_elemento);
    *desplazamiento += tamanio_elemento;
}

// void list_iterate(t_list *, void(*closure)(void*));



void copiar_y_desplazar_lista_strings_con_tamanios(void* paquete, t_list* lista_instrucciones, size_t* desplazamiento){
    // El vscode se enoja con esto pero funca igual.
    // La definí como función auxiliar para poder usar al paquete
    // Me gustaria mandar el paquete como parámetro pero list_iterate admite funciones con 1 solo parámetro.
    void copiar_y_desplazar_string_con_tamanio(void* string){
        size_t tamanio = strlen((char*)string) + 1;
        copiar_y_desplazar(paquete, &tamanio, sizeof(size_t), desplazamiento);
        copiar_y_desplazar(paquete, string, tamanio, desplazamiento);
    }

    list_iterate(lista_instrucciones, _f_aux);
}

// ------------------------------ SERIALIZACION INSTRUCCIONES ------------------------------ //

void* serializar_lista_instrucciones(size_t* size, t_list* lista_instrucciones) {

    // Hay 3 pasos. Calcular size total, hacer malloc, hacer memcpy de los datos a un stream y devolverlo.

    // Paso 1: Calcular size
    size_t size_payload = tamanio_lista_2(lista_instrucciones) + sizeof(size_t) * lista_instrucciones->elements_count;
    *size = sizeof(op_code) + sizeof(size_t) + size_payload;

    // Paso 2: Hacer malloc

    void* paquete = malloc(*size);

    // Paso 3: Hacer memcpy al paquete

    op_code codigo_operacion = 1; // COMO EJEMPLO, NO ES 1
    size_t* desplazamiento = malloc(sizeof(size_t)); *desplazamiento = 0;
    
    copiar_y_desplazar(paquete, &codigo_operacion, sizeof(op_code), desplazamiento);
    copiar_y_desplazar(paquete, &size_payload, sizeof(size_t), desplazamiento);
    
    // Recorrer la lista de instrucciones y para cada elemento hacer copiar_y_desplazar 2 veces, 1 de el size del string y otra del string en sí
    
    copiar_y_desplazar_lista_strings_con_tamanios(paquete, lista_instrucciones, desplazamiento);

    free(desplazamiento);

    // String pesa (strlen + 1) bytes, size_t pesa 8 bytes
    // printf("las instrucciones con sus tamanios pesan %ld", size_payload);
    // CHEQUEADO ANDA BIEN

    return paquete;

    /*
    tamanio_lista(size, lista_instrucciones);
    
    void* stream = malloc(*size);

    //serializo elemento x elemento

    int desp = 0;

    t_list_iterator* lista_it = list_iterator_create(lista_instrucciones);

    char* item = list_iterator_next(lista_it);

    printf("VOY A SERIALIZAR EL MENSAJE %s DE TAMANIO %ld", item, strlen(item));

    memcpy(stream + desp, &item, sizeof(item));



    for (int i=0; list_iterator_has_next(lista_it); i++) {
        char* item = list_iterator_next(lista_it);

        printf("VOY A SERIALIZAR EL MENSAJE %s DE TAMANIO %ld", item, sizeof(item));

        memcpy(stream + desp, &item, sizeof(item));
        desp+=sizeof(item);
    }
    list_iterator_destroy(lista_it);
    return stream;
    */
}


void deserializar_instrucciones(void* stream, int* numero){
    
    //aca estamos copiando el stream en la variable numero -> se recibe el mensaje
    memcpy(numero, stream, sizeof(int));                
}






// void* serializar_instrucciones(size_t* size, t_list* instrucciones) {
       
//     // stream lista de instrucciones

//     size_t size_instrucciones;
    
//     void* stream_instrucciones = serializar_lista_instrucciones(&size_instrucciones, instrucciones);

    
//     // stream completo

//     //size_t size_total =  sizeof(op_code) 
//      //                   + sizeof(size_t) // size total del stream
//      //                   + sizeof(size_t) // size instrucciones
//      //                   + size_instrucciones;   //instrucciones
    
   
//    //void* stream = malloc(size_total);

//     //op_code cop = INSTRUCCIONES;

   
//    /* size_t size_payload = *size - sizeof(op_code) - sizeof(size_t);

//     void* stream = malloc(*size);

//     op_code cop = INSTRUCCIONES;
//     memcpy(stream, &cop, sizeof(op_code));
//     memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
//     memcpy(stream + sizeof(op_code) + sizeof(size_t), &size_instrucciones, sizeof(size_t));
//     memcpy(stream + sizeof(op_code) + sizeof(size_t)*2 , instrucciones, size_instrucciones);
      
//       */
//     //return stream; 

//     //free(stream_instrucciones);
//     //*size = size_total;
//     //return stream;       
// }


//ESTAS DOS NO VAN EN ESTE .H

void tamanio_lista(size_t* size, t_list* lista) {

    t_list_iterator* lista_it = list_iterator_create(lista);
    for(int i = 0; list_iterator_has_next(lista_it); i++) {
        *size += (strlen(list_iterator_next(lista_it)));    //puede ser que tengamos que hacer +1 para el barra 0 ?
    }
    list_iterator_destroy(lista_it);
}

void* sumarSizeConLongitudString(void* a, void* b){
    size_t y = strlen((char*)b) + 1;
    *(size_t*)a += y;
    return a;
}

size_t tamanio_lista_2(t_list* lista){
    size_t* sumatoria = malloc(sizeof(size_t)); *sumatoria = 0;
    list_fold(lista, sumatoria, sumarSizeConLongitudString); // Devuelve lo foldeado pero hice que modificara sumatoria y listo, que al principio es la seed.
    size_t tamanio_lista = *sumatoria;
    free(sumatoria);
    return tamanio_lista;
}


// void mostrar_lista(t_list* lista) {

//     t_list_iterator* lista_it = list_iterator_create(lista);
//     for(int i = 0; list_iterator_has_next(lista_it); i++) {
//         printf("%s \n", list_iterator_next(lista_it));
//     }
//     list_iterator_destroy(lista_it);
// }


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










