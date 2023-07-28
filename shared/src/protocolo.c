#include "protocolo.h"

// ACA VAMOS A DEFNIR LAS FUNCIONES PARA ENVIAR Y RECIBIR MENSAJES ENTRE MODULOS
// Por cada mensaje tenemos un send, recv, serializar y deserializar


// ------------------------------ ENVIO Y RECEPCION DE NUMERO ------------------------------ //


bool send_numero(int fd, int numero) {
    //calculamos el tamanio del mensaje incluyendo el codigo de operacion 
    size_t size_paquete = sizeof(op_code) + sizeof(int);

    // mandamos los datos a un stream intermedio
    void* paquete = serializar_numero(numero);
    
    return send_paquete(fd, paquete, size_paquete);
}

bool recv_numero(int fd, int* numero) {
    void* paquete = recv_paquete(fd, sizeof(int));

    // deserializamos para guardar en la variable número el stream que recibimos.
    deserializar_numero(paquete, numero);
    free(paquete);
    return true;
}


// ------------------------------ ENVIO Y RECEPCION DE INSTRUCCIONES ------------------------------ //

bool send_instrucciones(int fd, t_list* lista_instrucciones) {
    size_t size_paquete = 0;
    void* paquete = serializar_instrucciones(&size_paquete, lista_instrucciones);
    
    return send_paquete(fd, paquete, size_paquete);
}


bool recv_instrucciones(int fd, t_list* instrucciones_recibidas){
    // Recibimos el size del payload
    size_t size_instrucciones;

    void* payload = recv_payload_con_size(fd, &size_instrucciones);

    size_t desplazamiento = 0;

    deserializar_instrucciones(payload, size_instrucciones, instrucciones_recibidas, &desplazamiento);

    free(payload);
    return true;
}

// ------------------------------ ENVIO Y RECEPCION DE CONTEXTO DE EJECUCION ------------------------------ //

bool send_contexto(int fd, t_contexto_ejecucion* contexto) {
    size_t size_paquete = 0;
    void* paquete = serializar_contexto(&size_paquete, contexto);
    
    return send_paquete(fd, paquete, size_paquete);
}


bool recv_contexto(int fd, t_contexto_ejecucion* contexto){
    // Recibimos el size del payload
    size_t size_contexto;
    
    
    void* payload = recv_payload_con_size(fd, &size_contexto);

    
    // Esto lo hacemos para que deserializar instrucciones se pueda usar en cualquier funcion  
    size_t desplazamiento = 0;

    //size_contexto = size_contexto - sizeof(int)*2 - sizeof(t_registros_cpu) - sizeof(size_t);
    deserializar_contexto(payload, size_contexto, contexto, &desplazamiento);
   

    free(payload);
    return true;
}



// ------------------------------ ENVIO Y RECEPCION DESALOJO ------------------------------ //

bool send_desalojo(int fd, int motivo_desalojo, t_list* lista_parametros) {
    size_t size_paquete = 0;
    void* paquete = serializar_desalojo(&size_paquete, motivo_desalojo, lista_parametros);
    
    return send_paquete(fd, paquete, size_paquete);
}

bool recv_desalojo(int fd, int* motivo_desalojo, t_list* lista_parametros) {
    size_t size_desalojo;
    
    void* payload = recv_payload_con_size(fd, &size_desalojo);

      
    size_t desplazamiento = 0;

    //size_contexto = size_contexto - sizeof(int)*2 - sizeof(t_registros_cpu) - sizeof(size_t);
    deserializar_desalojo(payload, size_desalojo, motivo_desalojo, lista_parametros, &desplazamiento);
   
    free(payload);
    return true;
}

// ------------------------------ ENVIO Y RECEPCION STRING ------------------------------ //

bool send_string(int fd, char* string){
    size_t size_paquete = 0;
    void* paquete = serializar_string(&size_paquete, string);

    return send_paquete(fd, paquete, size_paquete);
}

bool recv_string(int fd, char* string){
    size_t size_payload;
    
    void* payload = recv_payload_con_size(fd, &size_payload);

    size_t desplazamiento = 0;
    // deserializamos para guardar en la variable número el stream que recibimos.
    deserializar_string(payload, size_payload, string, &desplazamiento);
    // void deserializar_string(void* stream, size_t stream_size, char* string, size_t* desplazamiento)
    free(payload);
    return true;
}

// ------------------------------ ENVIO Y RECEPCION DE TABLA SEGMENTOS ------------------------------ //

bool send_tabla_segmentos(int fd, t_list* tabla_segmentos) {
    size_t size_paquete = 0;
    void* paquete = serializar_segmentos(&size_paquete, tabla_segmentos);
    
    return send_paquete(fd, paquete, size_paquete);
}


bool recv_tabla_segmentos(int fd, t_list* tabla_segmentos) {
    
    
    // Recibimos el size del payload
    size_t size_tabla_segmentos;

    void* payload = recv_payload_con_size(fd, &size_tabla_segmentos);

    size_t desplazamiento = 0;

    deserializar_segmentos(payload, size_tabla_segmentos, tabla_segmentos, &desplazamiento);

    //printf("RECIBI LA TABLA DE SEGMENTOS QUE ME MANDO MEMORIA");

    free(payload);
    return true;
}



bool send_solicitud_creacion_segmento(int fd, int pid, int id_segmento, int tamanio_segmento){
    size_t size_paquete = 0;
    void* paquete = serializar_solicitud_creacion_segmento(&size_paquete, pid, id_segmento, tamanio_segmento);
    
    return send_paquete(fd, paquete, size_paquete);
}

bool recv_solicitud_creacion_segmento(int fd, int* pid, int* id_segmento, int* tamanio_segmento) {
    // Recibimos el size del payload

    void* payload = recv_paquete(fd, sizeof(int) * 3);

    size_t desplazamiento = 0;

    deserializar_solicitud_creacion_segmento(payload, pid, id_segmento, tamanio_segmento, &desplazamiento);

    free(payload);
    return true;
}


bool send_solicitud_eliminacion_segmento(int fd, int id_segmento, int pid){
    size_t size_paquete = 0;
    void* paquete = serializar_solicitud_eliminacion_segmento(&size_paquete, id_segmento, pid);
    
    return send_paquete(fd, paquete, size_paquete);
}

bool recv_solicitud_eliminacion_segmento(int fd, int* id_segmento, int* pid) {
    // Recibimos el size del payload

    void* payload = recv_paquete(fd, sizeof(int) * 2);

    size_t desplazamiento = 0;

    deserializar_solicitud_eliminacion_segmento(payload, id_segmento, pid, &desplazamiento);

    free(payload);
    return true;
}


// ------------------------------ ENVIO Y RECEPCION DE TABLA POR PROCESO (COMPACTACION) ------------------------------ //

bool send_resultado_compactacion(int fd, t_list* tabla_segmentos_por_proceso, int cant_segmentos) {
    size_t size_paquete = 0;
    
    void* paquete = serializar_segmentos_por_proceso(&size_paquete, tabla_segmentos_por_proceso, cant_segmentos);
    
    return send_paquete(fd, paquete, size_paquete);
}

bool recv_resultado_compactacion(int fd, t_list* tabla_segmentos_por_proceso, int cant_segmentos) {
    
    // Recibimos el size del payload
    size_t size_tabla_segmentos_por_proceso;

    //printf("FALOPA1");

    void* payload = recv_payload_con_size(fd, &size_tabla_segmentos_por_proceso);

    //printf("FALOPA2");

    size_t desplazamiento = 0;

    deserializar_segmentos_por_proceso(payload, size_tabla_segmentos_por_proceso, tabla_segmentos_por_proceso, &desplazamiento, cant_segmentos);

    //printf("RECIBI LA TABLA DE SEGMENTOS QUE ME MANDO MEMORIA");

    free(payload);
    return true;
}



bool send_peticion_lectura(int fd, int direccion_fisica, int longitud){
    size_t size_paquete = 0;
    void* paquete = serializar_peticion_lectura(&size_paquete, direccion_fisica, longitud);
    
    return send_paquete(fd, paquete, size_paquete);
}

bool send_peticion_escritura(int fd, int direccion_fisica, int longitud, char* valor_leido){
    size_t size_paquete = 0;
    void* paquete = serializar_peticion_escritura(&size_paquete, direccion_fisica, longitud, valor_leido);
    
    return send_paquete(fd, paquete, size_paquete);
}


bool send_solicitud_tabla(int fd, int pid_counter) {
    //calculamos el tamanio del mensaje incluyendo el codigo de operacion 
    size_t size_paquete = sizeof(op_code) + sizeof(int);

    // mandamos los datos a un stream intermedio
    void* paquete = serializar_solicitud_tabla(pid_counter);
    
    return send_paquete(fd, paquete, size_paquete);
}


bool send_base_segmento_creado(int fd, int direccion_base){
    //calculamos el tamanio del mensaje incluyendo el codigo de operacion 
    size_t size_paquete = sizeof(int) + sizeof(int);

    // mandamos los datos a un stream intermedio
    void* paquete = serializar_base_segmento(direccion_base);
    
    return send_paquete(fd, paquete, size_paquete);
}

// OPCODE

void send_opcode(int fd, int opcode){
    SEND_INT(fd, opcode); // opcion 1: Lo malo es que no controla si hay error en send pero meh
    // send_paquete(fd, &opcode, sizeof(int)); // opcion 2
}

int recv_opcode(int fd){
    int cod_op;
    if(RECV_INT(fd, cod_op) > 0) return cod_op;
    
    //else
    close(fd);
    return -1;
}

// PAQUETE GENERICO

// Envia paquete generico, controlando errores
bool send_paquete(int fd, void* paquete, size_t size_paquete){
    if(send(fd, paquete, size_paquete, 0) != size_paquete) {
        free(paquete);
        return 0;
    }
    
    free(paquete);
    return 1;
}

// Dado el size recibe y retorna el paquete.
void* recv_paquete(int fd, size_t size_paquete){ 
    void* paquete = malloc(size_paquete);

    if(recv(fd, paquete, size_paquete, MSG_WAITALL) != size_paquete) { 
        free(paquete);
        return NULL;
    }

    return paquete;
}


// PAYLOAD CON SIZE

// Recibe size, crea espacio en memoria para el payload, recibe el payload, retorna el payload.
// Cuando la llamás no importa lo que valga size_payload antes.
// MODIFICA SIZE_PAYLOAD Y RETORNA PAYLOAD.
void* recv_payload_con_size(int fd, size_t* size_payload){
    size_t* size_aux = (size_t*)(recv_paquete(fd, sizeof(size_t)));
    *size_payload = *size_aux;
    free(size_aux);

    return recv_paquete(fd, *size_payload);
}