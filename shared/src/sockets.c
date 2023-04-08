#include "sockets.h"


/* ----------------------------------- SERVIDOR ----------------------------------- */

// INICIA SERVER ESCUCHANDO EN IP:PUERTO
int iniciar_servidor(char* ip, char* puerto, t_log* logger, const char* name) {
    
    int socket_servidor;
    struct addrinfo hints, *server_info;

    // Inicializando hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Recibe los addrinfo
    getaddrinfo(ip, puerto, &hints, &server_info);     

    
    // Se crea el socket de escucha del server
    socket_servidor = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    // Se asocia el socket a un puerto
    bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen);

    // Se escuchan las conexiones entrantes (hasta SOMAXCONN conexiones simultaneas)
    listen(socket_servidor, SOMAXCONN); 


    log_info(logger, "Escuchando en %s:%s (%s)\n", ip, puerto, name);

    freeaddrinfo(server_info);

    return socket_servidor;
}


// ESPERAR CONEXION DE CLIENTE EN UN SERVER ABIERTO
int esperar_cliente(int socket_servidor, t_log* logger, const char* name) {
    
    // Aceptamos un nuevo cliente
    int socket_cliente = accept(socket_servidor, NULL, NULL);

    log_info(logger, "Cliente conectado (a %s)\n", name);

    return socket_cliente;
}


int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}




/* ----------------------------------- CLIENTE ----------------------------------- */

// CLIENTE SE INTENTA CONECTAR A SERVER ESCUCHANDO EN IP:PUERTO
int crear_conexion(t_log* logger, const char* server_name, char* ip, char* puerto) {
    
    struct addrinfo hints, *servinfo;

    // Init de hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Recibe addrinfo
    getaddrinfo(ip, puerto, &hints, &servinfo);

    // Crea un socket con la informacion recibida (del primero, suficiente)
    int socket_cliente = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    // Fallo en crear el socket
    if(socket_cliente == -1) {
        log_error(logger, "Error creando el socket para %s:%s", ip, puerto);
        return 0;
    }

    // Error conectando
    if(connect(socket_cliente, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        log_error(logger, "Error al conectar (a %s)\n", server_name);
        freeaddrinfo(servinfo);
        return 0;
    } else
        log_info(logger, "Cliente conectado en %s:%s (a %s)\n", ip, puerto, server_name);

    freeaddrinfo(servinfo);

    return socket_cliente;
}

// CERRAR CONEXION
void liberar_conexion(int socket_cliente) {
    close(socket_cliente);
}




