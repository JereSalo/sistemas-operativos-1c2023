#include "serializacion.h"

// ------------------------------ SERIALIZACION INSTRUCCIONES ------------------------------ //

void* serializar_lista_instrucciones(size_t* size_instrucciones, t_list* lista_instrucciones) {
    // Hay 3 pasos. Calcular size total, hacer malloc, hacer memcpy de los datos a un stream y devolverlo.

    // Paso 1: Calcular size de lo que se quiere mandar, en este caso los strings de la lista y sus respectivos tamaños
    *size_instrucciones = tamanio_lista(lista_instrucciones) + sizeof(size_t) * lista_instrucciones->elements_count;
    
    // Paso 2: Hacer malloc
    void* stream = malloc(*size_instrucciones);

    // Paso 3: Guardar en stream los elementos de la lista con sus tamaños.

    // Recorrer la lista de instrucciones y para cada elemento hacer copiar_variable_en_stream_y_desplazar 2 veces, 1 de el size del string y otra del string en sí
    copiar_en_stream_y_desplazar_lista_strings_con_tamanios(stream, lista_instrucciones);
    
    return stream;
}

void* serializar_instrucciones(size_t* size, t_list* instrucciones) {
     size_t size_instrucciones;
    
     void* stream_instrucciones = serializar_lista_instrucciones(&size_instrucciones, instrucciones);

     // stream completo
     *size =    sizeof(op_code)
                + sizeof(size_t)        // size instrucciones
                + size_instrucciones;   // instrucciones
    
   
    void* paquete = malloc(*size);
    
    op_code codigo_operacion = INSTRUCCIONES;
  
    size_t desplazamiento = 0;

    copiar_variable_en_stream_y_desplazar(paquete, &codigo_operacion, sizeof(op_code), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &size_instrucciones, sizeof(size_t), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, stream_instrucciones, size_instrucciones, &desplazamiento);
           
    free(stream_instrucciones);
    return paquete;
}

// Dado un stream (con instrucciones) y una lista de instrucciones (vacia), la armamos con los elementos del stream.
void deserializar_instrucciones(void* stream, size_t stream_size , t_list* instrucciones_recibidas, size_t desplazamiento){
    // Tenemos todo el stream con los elementos y sus tamaños.
   
    while(desplazamiento < stream_size){ 
        size_t tamanio_string = 0;
        copiar_stream_en_variable_y_desplazar(&tamanio_string, stream, sizeof(size_t), &desplazamiento);
        char* string = malloc(tamanio_string);
        copiar_stream_en_variable_y_desplazar(string, stream, tamanio_string, &desplazamiento);

        list_add(instrucciones_recibidas, string);
        // WARNING: NO HACER FREE DEL STRING, SE LIBERA DESPUÉS CUANDO DESTRUIMOS LA LISTA :)
    }
}


// ------------------------------ SERIALIZACION CONTEXTO DE EJECUCION ------------------------------ //

// ACA VAMOS A SERIALIZAR TODO EL CONTEXTO MENOS LOS REGISTROS
void* serializar_contexto(size_t* size, t_contexto_ejecucion* contexto) {

    size_t size_instrucciones;

    void* stream_instrucciones = serializar_lista_instrucciones(&size_instrucciones, contexto->instrucciones);

     // stream completo
     *size =    sizeof(op_code)
                + sizeof(size_t)            //size total del payload
                + sizeof(int)               //pid
                + sizeof(int)               //pc
                + sizeof(t_registros_cpu)   // registros cpu
                + size_instrucciones;       // instrucciones
    
    //ERA AL PEDO MANDAR PRIMERO EL SIZE DE INSTRUCCIONES Y DESPUES LAS INSTRUCCIONES
    //SOLO DEBEMOS MANDAR EL SIZE DE TODO EL PAYLOAD UNA SOLA VEZ
    
    size_t size_payload = *size - sizeof(op_code) - sizeof(size_t);
   
    void* paquete = malloc(*size);
    
    op_code codigo_operacion = CONTEXTO_EJECUCION;
  
    size_t desplazamiento = 0;

    copiar_variable_en_stream_y_desplazar(paquete, &codigo_operacion, sizeof(op_code), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &size_payload, sizeof(size_t), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &contexto->pid, sizeof(int), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &contexto->pc, sizeof(int), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, contexto->registros_cpu, sizeof(t_registros_cpu), &desplazamiento);
    //copiar_variable_en_stream_y_desplazar(paquete, &size_instrucciones, sizeof(size_t), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, stream_instrucciones, size_instrucciones, &desplazamiento);
           
    free(stream_instrucciones);
    return paquete;
}

void deserializar_contexto(void* stream, size_t stream_size, t_contexto_ejecucion* contexto, size_t desplazamiento) {

    //size_t size_instrucciones = 0;

    copiar_stream_en_variable_y_desplazar(&contexto->pid, stream, sizeof(int), &desplazamiento);
    copiar_stream_en_variable_y_desplazar(&contexto->pc, stream, sizeof(int), &desplazamiento);
    copiar_stream_en_variable_y_desplazar(contexto->registros_cpu, stream, sizeof(t_registros_cpu), &desplazamiento);
    //copiar_stream_en_variable_y_desplazar(&size_instrucciones, stream, sizeof(size_t), &desplazamiento);
    
    contexto->instrucciones = list_create();
        
    deserializar_instrucciones(stream, stream_size, contexto->instrucciones, desplazamiento);
}




































// ------------------------------ SERIALIZACION NUMERO (PRUEBA) ------------------------------ //

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