#include "protocolo.h"

// ACA VAMOS A DEFNIR LAS FUNCIONES PARA ENVIAR Y RECIBIR MENSAJES ENTRE MODULOS


// Por cada mensaje tenemos un send, recv, serializar y deserializar


// ------------------------------ ENVIAR ------------------------------ //

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


bool send_numero(int fd, int numero) {
    
    //calculamos el tamanio del mensaje incluyendo el codigo de operacion 
    size_t size = sizeof(op_code) + sizeof(int);

    // mandamos los datos a un stream intermedio
    void* stream = serializar_numero(numero);
    
    //mandamos los datos copiados en ese stream al destinatario
    if(send(fd, stream, size, 0) != size) {     //send retorna el tamanio que se envio
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

// ------------------------------ RECIBIR ------------------------------ //


bool recv_numero(int fd, int* numero) {
    
    //calculamos el tamanio de SOLO el payload
    size_t size = sizeof(numero);

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

void deserializar_numero(void* stream, int* numero) {
    
    //aca estamos copiando el stream en la variable numero -> se recibe el mensaje
    memcpy(numero, stream, sizeof(int));                
}


// ------------------------------ PROCESAR ------------------------------ //

void procesar_conexion(t_log* logger, int cliente_socket, char* server_name) {
    /*
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
    t_log* logger = args->log;
    int cliente_socket = args->fd;
    char* server_name = args->server_name;
    free(args);
    */

    op_code codigo;
    while(cliente_socket != -1) {
        
        // Recibís el código de operación
        if(recv(cliente_socket, &codigo, sizeof(op_code), 0) != sizeof(op_code)) {
            log_error(logger, "ERROR FATAL");
            return;
        }

        // Depende el codigo de operacion vamos a hacer una cosa u otra.
        // Aca el código de operación ya lo recibiste, entonces lo único que queda por recibir es el payload (o sea, el mensaje :D)
        switch(codigo) {
            case NUMERO:
            {
                int numero_recibido;

                if(!recv_numero(cliente_socket, &numero_recibido)) {
                    log_error(logger, "Fallo recibiendo NUMERO");
                    break;
                }

                log_info(logger, "RECIBI EL MENSAJE %d", numero_recibido);
            }
        }
    }
}


