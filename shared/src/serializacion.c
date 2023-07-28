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
void deserializar_instrucciones(void* stream, size_t size_instrucciones , t_list* instrucciones_recibidas, size_t* desplazamiento){
    // Tenemos todo el stream con los elementos y sus tamaños.
    size_t desplazamiento_inicial = *desplazamiento;

    while(*desplazamiento < size_instrucciones + desplazamiento_inicial){
        size_t tamanio_string = 0;
        copiar_stream_en_variable_y_desplazar(&tamanio_string, stream, sizeof(size_t), desplazamiento);
        char* string = malloc(tamanio_string);
        copiar_stream_en_variable_y_desplazar(string, stream, tamanio_string, desplazamiento);

        list_add(instrucciones_recibidas, string);
        // WARNING: NO HACER FREE DEL STRING, SE LIBERA DESPUÉS CUANDO DESTRUIMOS LA LISTA :)
    }
}

// ------------------------------ SERIALIZACION CONTEXTO DE EJECUCION ------------------------------ //

void* serializar_contexto(size_t* size, t_contexto_ejecucion* contexto) {

    size_t size_instrucciones;
    size_t size_segmentos;

    void* stream_instrucciones = serializar_lista_instrucciones(&size_instrucciones, contexto->instrucciones);
    void* stream_segmentos = serializar_tabla_segmentos(&size_segmentos, contexto->tabla_segmentos);

     // stream completo
     *size =    sizeof(op_code)
                + sizeof(size_t)            // size total del payload
                + sizeof(int)               // pid
                + sizeof(int)               // pc
                + sizeof(t_registros_cpu)   // registros cpu
                + sizeof(size_t)            // size instrucciones
                + size_instrucciones        // instrucciones
                + sizeof(size_t)            // size segmentos
                + size_segmentos;           // segmentos
    
    size_t size_payload = *size - sizeof(op_code) - sizeof(size_t);
   
    void* paquete = malloc(*size);
    
    op_code codigo_operacion = CONTEXTO_EJECUCION;
  
    size_t desplazamiento = 0;

    copiar_variable_en_stream_y_desplazar(paquete, &codigo_operacion, sizeof(op_code), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &size_payload, sizeof(size_t), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &contexto->pid, sizeof(int), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &contexto->pc, sizeof(int), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, contexto->registros_cpu, sizeof(t_registros_cpu), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &size_instrucciones, sizeof(size_t), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, stream_instrucciones, size_instrucciones, &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &size_segmentos, sizeof(size_t), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, stream_segmentos, size_segmentos, &desplazamiento);
    
    free(stream_instrucciones);
    free(stream_segmentos);
    return paquete;
}

void deserializar_contexto(void* stream, size_t stream_size, t_contexto_ejecucion* contexto, size_t* desplazamiento) {
    size_t size_instrucciones;
    size_t size_segmentos;

    copiar_stream_en_variable_y_desplazar(&contexto->pid, stream, sizeof(int), desplazamiento);

    copiar_stream_en_variable_y_desplazar(&contexto->pc, stream, sizeof(int), desplazamiento);

    copiar_stream_en_variable_y_desplazar(contexto->registros_cpu, stream, sizeof(t_registros_cpu), desplazamiento);

    copiar_stream_en_variable_y_desplazar(&size_instrucciones, stream, sizeof(size_t), desplazamiento);

    deserializar_instrucciones(stream, size_instrucciones, contexto->instrucciones, desplazamiento);

    copiar_stream_en_variable_y_desplazar(&size_segmentos, stream, sizeof(size_t), desplazamiento);

    deserializar_segmentos(stream, size_segmentos, contexto->tabla_segmentos, desplazamiento);
}



// Serializa el motivo de desalojo y la lista de parametros
void* serializar_desalojo(size_t* size, int motivo_desalojo, t_list* lista_parametros) {

    size_t size_parametros;

    void* stream_parametros = serializar_lista_instrucciones(&size_parametros, lista_parametros);

     // stream completo
     *size =    sizeof(op_code)
                + sizeof(size_t)            //size total del payload
                + sizeof(int)               //motivo de desalojo
                + sizeof(size_t)            //size parametros
                + size_parametros;         // parametros
    
    size_t size_payload = *size - sizeof(op_code) - sizeof(size_t);
   
    void* paquete = malloc(*size);
    
    op_code codigo_operacion = PROCESO_DESALOJADO;
  
    size_t desplazamiento = 0;

    copiar_variable_en_stream_y_desplazar(paquete, &codigo_operacion, sizeof(op_code), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &size_payload, sizeof(size_t), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &motivo_desalojo, sizeof(int), &desplazamiento); //CHEQUEAR
    copiar_variable_en_stream_y_desplazar(paquete, &size_parametros, sizeof(size_t), &desplazamiento); 
    copiar_variable_en_stream_y_desplazar(paquete, stream_parametros, size_parametros, &desplazamiento);

    free(stream_parametros);
    return paquete;

}

void deserializar_desalojo(void* stream, size_t stream_size, int* motivo_desalojo, t_list* lista_parametros, size_t* desplazamiento) {
    
    size_t size_parametros;

    //printf("MOTIVO DE DESALOJO: %d", *motivo_desalojo);
    copiar_stream_en_variable_y_desplazar(motivo_desalojo, stream, sizeof(int), desplazamiento);

    copiar_stream_en_variable_y_desplazar(&size_parametros, stream, sizeof(size_t), desplazamiento);

    deserializar_instrucciones(stream, size_parametros, lista_parametros, desplazamiento);
}



// Serializar finalizacion

void* serializar_string(size_t* size, char* string) {
    size_t size_string = strlen(string) + 1;

    *size = size_string + sizeof(size_t);
    
    void* paquete = malloc(*size);

    size_t desplazamiento = 0;

    copiar_variable_en_stream_y_desplazar(paquete, &size_string, sizeof(size_t), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, string, size_string, &desplazamiento);
    
    return paquete;
}



void deserializar_string(void* stream, size_t stream_size, char* string, size_t* desplazamiento) {
    copiar_stream_en_variable_y_desplazar(string, stream, stream_size, desplazamiento);
}

// recv del size_t
// recv de algo del tamaño de lo de arriba


// ------------------------------ SERIALIZACION TABLA SEGMENTOS ------------------------------ //

void* serializar_tabla_segmentos(size_t* size_tabla_segmentos, t_list* tabla_segmentos) {
       
    *size_tabla_segmentos = sizeof(t_segmento) * list_size(tabla_segmentos); 
        
    void* stream = malloc(*size_tabla_segmentos);

    size_t desplazamiento = 0;

    t_list_iterator* lista_it = list_iterator_create(tabla_segmentos);

    while (list_iterator_has_next(lista_it)) {
        t_segmento* segmento = (t_segmento*)list_iterator_next(lista_it);
        
        copiar_variable_en_stream_y_desplazar(stream, &(segmento->id), sizeof(int), &desplazamiento);
        copiar_variable_en_stream_y_desplazar(stream, &(segmento->direccion_base), sizeof(int), &desplazamiento);
        copiar_variable_en_stream_y_desplazar(stream, &(segmento->tamanio), sizeof(int), &desplazamiento);    

    }
    
    list_iterator_destroy(lista_it);
    return stream;
}
    
    
void* serializar_segmentos(size_t* size, t_list* segmentos) {
    size_t size_segmentos;
    
    void* stream_tabla_segmentos = serializar_tabla_segmentos(&size_segmentos, segmentos);


    // stream completo
    *size = sizeof(size_t)         // size segmentos
            + size_segmentos;      // segmentos
    
   
    void* paquete = malloc(*size);

   
    size_t desplazamiento = 0;

    copiar_variable_en_stream_y_desplazar(paquete, &size_segmentos, sizeof(size_t), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, stream_tabla_segmentos, size_segmentos, &desplazamiento);
  
    free(stream_tabla_segmentos);
    return paquete;
}

// Dado un stream (con instrucciones) y una lista de instrucciones (vacia), la armamos con los elementos del stream.
void deserializar_segmentos(void* stream, size_t size_segmentos , t_list* tabla_segmentos, size_t* desplazamiento){
    // Tenemos todo el stream con los elementos y sus tamaños.
    size_t desplazamiento_inicial = *desplazamiento;

    while(*desplazamiento < size_segmentos + desplazamiento_inicial){
        
        t_segmento* segmento = malloc(sizeof(t_segmento));
        copiar_stream_en_variable_y_desplazar(&(segmento->id), stream, sizeof(int), desplazamiento);
        copiar_stream_en_variable_y_desplazar(&(segmento->direccion_base), stream, sizeof(int), desplazamiento);
        copiar_stream_en_variable_y_desplazar(&(segmento->tamanio), stream, sizeof(int), desplazamiento);


        list_add(tabla_segmentos, segmento);
        // WARNING: NO HACER FREE DEL STRING, SE LIBERA DESPUÉS CUANDO DESTRUIMOS LA LISTA :)
    }
}

void* serializar_solicitud_tabla(int pid_counter){
    void* paquete = malloc(sizeof(op_code) + sizeof(int));

    size_t desplazamiento = 0;
    
    op_code cop = SOLICITUD_TABLA_NEW;

    copiar_variable_en_stream_y_desplazar(paquete, &cop, sizeof(op_code), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &pid_counter, sizeof(int), &desplazamiento);
    
    return paquete;
}

void* serializar_base_segmento(int direccion_base){
    void* paquete = malloc(sizeof(int) + sizeof(int));

    size_t desplazamiento = 0;
    
    int cop = CREACION;

    copiar_variable_en_stream_y_desplazar(paquete, &cop, sizeof(int), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &direccion_base, sizeof(int), &desplazamiento);
    
    return paquete;
}




void* serializar_solicitud_creacion_segmento(size_t* size, int pid, int id_segmento, int tamanio_segmento){
    // stream completo
    *size = sizeof(op_code) +
            sizeof(int) * 3;      // PID, ID_SEGMENTO, TAMANIO_SEGMENTO
    
    void* paquete = malloc(*size);

    size_t desplazamiento = 0;
    
    op_code cop = SOLICITUD_CREACION_SEGMENTO;

    copiar_variable_en_stream_y_desplazar(paquete, &cop, sizeof(op_code), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &pid, sizeof(int), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &id_segmento, sizeof(int), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &tamanio_segmento, sizeof(int), &desplazamiento);
  
    return paquete;
}


void deserializar_solicitud_creacion_segmento(void* payload, int* pid, int* id_segmento, int* tamanio_segmento, size_t* desplazamiento){
    copiar_stream_en_variable_y_desplazar(pid, payload, sizeof(int), desplazamiento);
    copiar_stream_en_variable_y_desplazar(id_segmento, payload, sizeof(int), desplazamiento);
    copiar_stream_en_variable_y_desplazar(tamanio_segmento, payload, sizeof(int), desplazamiento);
}


void* serializar_solicitud_eliminacion_segmento(size_t* size, int id_segmento, int pid){
    // stream completo
    *size = sizeof(op_code) +
            sizeof(int) * 2;      // DIRECCION_BASE, ID_SEGMENTO, TAMANIO_SEGMENTO
    
    void* paquete = malloc(*size);

    size_t desplazamiento = 0;
    
    op_code cop = SOLICITUD_ELIMINACION_SEGMENTO;

    copiar_variable_en_stream_y_desplazar(paquete, &cop, sizeof(op_code), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &id_segmento, sizeof(int), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &pid, sizeof(int), &desplazamiento);
  
    return paquete;
}


void deserializar_solicitud_eliminacion_segmento(void* payload, int* id_segmento, int* pid, size_t* desplazamiento){
    copiar_stream_en_variable_y_desplazar(id_segmento, payload, sizeof(int), desplazamiento);
    copiar_stream_en_variable_y_desplazar(pid, payload, sizeof(int), desplazamiento);
}


// ------------------------------ SERIALIZACION DE TABLA POR PROCESO (COMPACTACION) ------------------------------ //

void* serializar_segmentos_por_proceso(size_t* size, t_list* tabla_segmentos_por_proceso, int cant_segmentos) {
    size_t size_segmentos_por_proceso;
    
    void* stream_tabla_segmentos_por_proceso = serializar_tabla_segmentos_por_proceso(&size_segmentos_por_proceso, tabla_segmentos_por_proceso, cant_segmentos);

    // stream completo
    *size = sizeof(size_t)         		       // size segmentos_por_proceso
            + size_segmentos_por_proceso;      // segmentos_por_proceso
    
   
    void* paquete = malloc(*size);
   
    size_t desplazamiento = 0;

    copiar_variable_en_stream_y_desplazar(paquete, &size_segmentos_por_proceso, sizeof(size_t), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, stream_tabla_segmentos_por_proceso, size_segmentos_por_proceso, &desplazamiento);
  
    free(stream_tabla_segmentos_por_proceso);
    return paquete;
}



void* serializar_tabla_segmentos_por_proceso(size_t* size_segmentos_por_proceso, t_list* tabla_segmentos_por_proceso, int cant_segmentos) {
    
    // El size de la tabla de segmentos por proceso incluye al pid del proceso + el tamanio de cada tabla de segmentos
    // Esto se multiplica por la cantidad de entradas que tenga la tabla (size)
    
    size_t size_tabla_segmentos = sizeof(t_segmento) * cant_segmentos; 
    
    *size_segmentos_por_proceso = (sizeof(int) + size_tabla_segmentos) * list_size(tabla_segmentos_por_proceso);    

    void* stream = malloc(*size_segmentos_por_proceso);

    size_t desplazamiento = 0;

    t_list_iterator* lista_it = list_iterator_create(tabla_segmentos_por_proceso);
    
    while (list_iterator_has_next(lista_it)) {
        t_tabla_proceso* tabla_proceso = (t_tabla_proceso*)list_iterator_next(lista_it);
           
        // Aca reusamos la funcion de serializar una tabla en particular
        size_t size_segmentos;    
    
        void* stream_tabla_segmentos = serializar_tabla_segmentos(&size_segmentos, tabla_proceso->lista_segmentos);
        
        copiar_variable_en_stream_y_desplazar(stream, &(tabla_proceso->pid), sizeof(int), &desplazamiento);
        copiar_variable_en_stream_y_desplazar(stream, stream_tabla_segmentos, size_segmentos, &desplazamiento);
        
        // La serialiacion queda PID | TABLA SEGMENTOS y esto se hace por cada elemento

        free(stream_tabla_segmentos);
    }
    
    list_iterator_destroy(lista_it);
    return stream; 
}


void deserializar_segmentos_por_proceso(void* stream, size_t size_segmentos , t_list* tabla_segmentos_por_proceso, size_t* desplazamiento, int cant_segmentos) {

    // Tenemos todo el stream con los elementos y sus tamaños.
    
    size_t desplazamiento_inicial = *desplazamiento;
    
    // Una tabla de segmentos siempre va a pesar esto
    size_t size_tabla = sizeof(t_segmento) * cant_segmentos;  


    while(*desplazamiento < size_segmentos + desplazamiento_inicial){
        
        t_tabla_proceso* tabla_proceso = malloc(sizeof(t_tabla_proceso));

        tabla_proceso->lista_segmentos = list_create();
        
        copiar_stream_en_variable_y_desplazar(&(tabla_proceso->pid), stream, sizeof(int), desplazamiento);
        
        deserializar_segmentos(stream, size_tabla, tabla_proceso->lista_segmentos, desplazamiento);

        list_add(tabla_segmentos_por_proceso, tabla_proceso);
    }
}




void* serializar_peticion_lectura(size_t* size, int direccion_fisica, int longitud){
    
    // stream completo
    *size = sizeof(op_code) +
            sizeof(int) * 2;      // DIRECCION_FISICA, LONGITUD
    
    void* paquete = malloc(*size);

    size_t desplazamiento = 0;
    
    op_code cop = SOLICITUD_LECTURA;

    copiar_variable_en_stream_y_desplazar(paquete, &cop, sizeof(op_code), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &direccion_fisica, sizeof(int), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &longitud, sizeof(int), &desplazamiento);
  
    return paquete;
}

void* serializar_peticion_escritura(size_t* size, int direccion_fisica, int longitud, char* valor_leido){
    
    // stream completo
    *size = sizeof(op_code) +
            sizeof(int) * 2 + // DIRECCION_FISICA, LONGITUD
            longitud;      
    
    void* paquete = malloc(*size);

    size_t desplazamiento = 0;
    
    op_code cop = SOLICITUD_ESCRITURA;

    copiar_variable_en_stream_y_desplazar(paquete, &cop, sizeof(op_code), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &direccion_fisica, sizeof(int), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, &longitud, sizeof(int), &desplazamiento);
    copiar_variable_en_stream_y_desplazar(paquete, valor_leido, longitud, &desplazamiento);
  
    return paquete;
}



// ------------------------------ SERIALIZACION NUMERITO (PRUEBA) ------------------------------ //

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