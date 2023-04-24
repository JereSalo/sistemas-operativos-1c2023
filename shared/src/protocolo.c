#include "protocolo.h"

// ACA VAMOS A DEFNIR LAS FUNCIONES PARA ENVIAR Y RECIBIR MENSAJES ENTRE MODULOS
// Por cada mensaje tenemos un send, recv, serializar y deserializar


// ------------------------------ ENVIO Y RECEPCION DE NUMERO ------------------------------ //


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






// Hacer un procesar conexion por cada cliente -> esto va a ser util para los servers que tengan varios clientes
// Asi nos evitamos que el switch quede muy grande, ya que no todos los servers van a entender los mismos mensajes

// ------------------------------ PROCESAR ------------------------------ //

void procesar_conexion(void* void_args) {
    
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
    t_log* logger = args->log;      
    int cliente_socket = args->fd;    
    char* server_name = args->server_name;

    free(args);
    
    op_code codigo;
    while(cliente_socket != -1) {
        
        // Recibís el código de operación
        if(recv(cliente_socket, &codigo, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(logger, "CLIENTE DESCONECTADO");
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
            case INSTRUCCIONES:
            {

                
            }
            
        }
    }
}