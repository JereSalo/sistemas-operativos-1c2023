#include "serializacion.h"


void copiar_y_desplazar(void* paquete, void* elemento, size_t tamanio_elemento, size_t* desplazamiento){
    memcpy(paquete + *desplazamiento, elemento, tamanio_elemento);
    *desplazamiento += tamanio_elemento;
}

// void list_iterate(t_list *, void(*closure)(void*));



void copiar_y_desplazar_lista_strings_con_tamanios(void* paquete, t_list* lista_instrucciones){
    // El vscode se enoja con esto pero funca igual.
    // La definí como función auxiliar para poder usar al paquete
    // Me gustaria mandar el paquete como parámetro pero list_iterate admite funciones con 1 solo parámetro.
    size_t desplazamiento = 0;
    
    void copiar_y_desplazar_string_con_tamanio(void* string) {
        size_t tamanio = strlen((char*)string) + 1;
        copiar_y_desplazar(paquete, &tamanio, sizeof(size_t), &desplazamiento);
        copiar_y_desplazar(paquete, string, tamanio, &desplazamiento);
    }

    list_iterate(lista_instrucciones, copiar_y_desplazar_string_con_tamanio);
}

// ------------------------------ SERIALIZACION INSTRUCCIONES ------------------------------ //

void* serializar_lista_instrucciones(size_t* size, t_list* lista_instrucciones) {
    // Hay 3 pasos. Calcular size total, hacer malloc, hacer memcpy de los datos a un stream y devolverlo.

    // Paso 1: Calcular size de la lista
    *size = tamanio_lista(lista_instrucciones) + sizeof(size_t) * lista_instrucciones->elements_count;
    

    // Paso 2: Hacer malloc
    void* stream = malloc(*size);

    
    // Recorrer la lista de instrucciones y para cada elemento hacer copiar_y_desplazar 2 veces, 1 de el size del string y otra del string en sí
    copiar_y_desplazar_lista_strings_con_tamanios(stream, lista_instrucciones);

    // String pesa (strlen + 1) bytes, size_t pesa 8 bytes
    printf("las instrucciones con sus tamanios pesan %ld", *size);
    // CHEQUEADO ANDA BIEN

    return stream;
}

void* serializar_instrucciones(size_t* size, t_list* instrucciones) {

     // stream lista de instrucciones

     size_t size_instrucciones;
    
     void* stream_instrucciones = serializar_lista_instrucciones(&size_instrucciones, instrucciones);

     // stream completo
     *size =    sizeof(op_code) // SE PODRIA AGREGAR CANTIDAD DE ELEMENTOS DE LA LISTA PARA ENVIAR
                + sizeof(size_t)        // size instrucciones
                + size_instrucciones;   //instrucciones
    
   
    void* paquete = malloc(*size);
    
    op_code codigo_operacion = INSTRUCCIONES;
  
    size_t desplazamiento = 0;

    copiar_y_desplazar(paquete, &codigo_operacion, sizeof(op_code), &desplazamiento);
    copiar_y_desplazar(paquete, &size_instrucciones, sizeof(size_t), &desplazamiento);
    copiar_y_desplazar(paquete, stream_instrucciones, size_instrucciones, &desplazamiento);
           
    free(stream_instrucciones);
    return paquete;
}

void deserializar_instrucciones(void* stream, size_t stream_size , t_list* instrucciones_recibidas){
    // Tenemos todo el stream con los elementos y sus tamaños.
    size_t desplazamiento = 0;

    while(desplazamiento < stream_size){        
        size_t tamanio_string;

        memcpy(&tamanio_string,stream + desplazamiento,sizeof(size_t));
        desplazamiento += sizeof(size_t);
        
        char* string = malloc(tamanio_string);
        
        memcpy((void*)string,stream + desplazamiento,tamanio_string);
        desplazamiento += tamanio_string;

        list_add(instrucciones_recibidas, string);
    }        
}




// ESTAS SON FUNCIONES AUXILIARES, NO VAN ACA. VER DE PONERLAS EN OTRO ARCHIVO

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