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

bool send_instrucciones(int fd, t_list* lista_instrucciones) {
    size_t size = 0;
    void* paquete = serializar_instrucciones(&size, lista_instrucciones);
    
    //mandamos los datos copiados en ese stream al destinatario
    if(send(fd, paquete, size, 0) != size) {     //send retorna el tamanio que se envio
        printf("Hubo un error con el send\n");
        free(paquete);
        return false;
    }
    // Aca semaforo para decirle al kernel que ya puede recibir las instrucciones?
    free(paquete);
    return true;
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
    
    size_t size = 0;
    void* paquete = serializar_contexto(&size, contexto);
    
    //mandamos los datos copiados en ese stream al destinatario
    if(send(fd, paquete, size, 0) != size) {     //send retorna el tamanio que se envio
        printf("Hubo un error con el send\n");
        free(paquete);
        return false;
    }
    
    free(paquete);
    return true;
}


bool recv_contexto(int fd, t_contexto_ejecucion* contexto){
    contexto->instrucciones = list_create();
    contexto->registros_cpu = malloc(sizeof(t_registros_cpu));

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





// Hacer un procesar conexion por cada cliente -> esto va a ser util para los servers que tengan varios clientes
// Asi nos evitamos que el switch quede muy grande, ya que no todos los servers van a entender los mismos mensajes


// ------------------------------ ENVIO Y RECEPCION DE STRINGCITO ------------------------------ //

/* bool send_string(int fd, char* string) {
    
    size_t size = 0;
    void* paquete = serializar_string(&size, string);
    
    //mandamos los datos copiados en ese stream al destinatario
    if(send(fd, paquete, size, 0) != size) {     //send retorna el tamanio que se envio
        printf("Hubo un error con el send\n");
        free(paquete);
        return false;
    }
    
    free(paquete);
    return true;
}


bool recv_string(int fd, char** string){
    // Recibimos el size del payload
    

    size_t size_string;
    //printf("Intento recibir size del payload\n");
    if (recv(fd, &size_string, sizeof(size_t), 0) != sizeof(size_t)){
        printf("Fallo recibiendo size del payload\n");
        return false;
    }

    // Hacemos malloc para poder guardar todo el payload
    void* stream = malloc(size_string);

    

    // Recibimos todo el payload
    //printf("Intento recibir todo el payload\n");
    if (recv(fd, stream, size_string, 0) != size_string){
        printf("Fallo al recibir todo el payload\n");
        free(stream);
        return false;
    }

    // Esto lo hacemos para que deserializar instrucciones se pueda usar en cualquier funcion
      
    size_t desplazamiento = 0;

    //size_contexto = size_contexto - sizeof(int)*2 - sizeof(t_registros_cpu) - sizeof(size_t);

    deserializar_string(stream, &string);


    free(stream);
    return true;
} */