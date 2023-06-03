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
    
    //calculamos el tamanio de SOLO el payload
    size_t size = sizeof(int);

    //creamos un stream intermedio para guardar el mensaje que vamos a recibir
    void* stream = malloc(size);

    // en recv se modifica la variable stream, guardando ahi lo recibido.
    if(recv(fd, stream, size, 0) != size) { 
        free(stream);
        return false;
    }

    // deserializamos para guardar en la variable número el stream que recibimos.
    deserializar_numero(stream, numero);
    free(stream);
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
    //printf("Intento recibir size del payload\n");
    if (recv(fd,&size_instrucciones, sizeof(size_t), 0) != sizeof(size_t)){
        printf("Fallo recibiendo size del payload\n");
        return false;
    }

    // Hacemos malloc para poder guardar todo el payload
    void* stream = malloc(size_instrucciones);

    // Recibimos todo el payload
    //printf("Intento recibir todo el payload\n");
    if (recv(fd, stream, size_instrucciones, 0) != size_instrucciones){
        printf("Fallo al recibir todo el payload\n");
        free(stream);
        return false;
    }

    size_t desplazamiento = 0;

    deserializar_instrucciones(stream, size_instrucciones, instrucciones_recibidas, &desplazamiento);

    free(stream);
    return true;
}

// ------------------------------ ENVIO Y RECEPCION DE CONTEXTO DE EJECUCION ------------------------------ //

bool send_contexto(int fd, t_contexto_ejecucion* contexto) {
    size_t size_paquete = 0;
    void* paquete = serializar_contexto(&size_paquete, contexto);
    
    return send_paquete(fd, paquete, size_paquete);
}


bool recv_contexto(int fd, t_contexto_ejecucion* contexto){
    contexto->instrucciones = list_create();
    contexto->registros_cpu = malloc(sizeof(t_registros_cpu));
    contexto->pid = malloc(sizeof(int));

    // Recibimos el size del payload
    size_t size_contexto;
    
    //printf("Intento recibir size del payload\n");
    if (recv(fd, &size_contexto, sizeof(size_t), 0) != sizeof(size_t)){
        printf("Fallo recibiendo size del payload\n");
        return false;
    }

    // Hacemos malloc para poder guardar todo el payload
    void* stream = malloc(size_contexto);

    // Recibimos todo el payload
    // printf("Intento recibir todo el payload\n");
    if (recv(fd, stream, size_contexto, 0) != size_contexto){
        printf("Fallo al recibir todo el payload\n");
        free(stream);
        return false;
    }

    // Esto lo hacemos para que deserializar instrucciones se pueda usar en cualquier funcion
      
    size_t desplazamiento = 0;

    //size_contexto = size_contexto - sizeof(int)*2 - sizeof(t_registros_cpu) - sizeof(size_t);
    deserializar_contexto(stream, size_contexto, contexto, &desplazamiento);
   


    free(stream);
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
    
    void* payload = recv_payload_con_size(fd, size_desalojo);

      
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