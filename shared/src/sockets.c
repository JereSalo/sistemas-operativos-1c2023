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


// Hace lo que dice. Inicia el servidor, espera que el cliente se conecte. Cuando se conecta devuelve el socket con la conexión.
int iniciar_servidor_y_esperar_cliente(int modulo, t_config *config, t_log *logger){ 
    char* nombre_modulo;
    char* ip;
    char* puerto;

    switch (modulo)
    {
    case KERNEL:
        ip = config_get_string_value(config, "IP_KERNEL");
        puerto = config_get_string_value(config, "PUERTO_ESCUCHA");
        nombre_modulo = strdup("Kernel");
        break;
    case CPU:
        ip = config_get_string_value(config, "IP_CPU");
        puerto = config_get_string_value(config, "PUERTO_CPU");
        nombre_modulo = strdup("CPU");
        break;
    case MEMORIA:
        ip = config_get_string_value(config, "IP_MEMORIA");
        puerto = config_get_string_value(config, "PUERTO_MEMORIA");
        nombre_modulo = strdup("Memoria");
        break;
    case FILESYSTEM:
        ip = config_get_string_value(config, "IP_FILESYSTEM");
        puerto = config_get_string_value(config, "PUERTO_FILESYSTEM");
        nombre_modulo = strdup("FileSystem");
        break;
    }

    int server_fd = iniciar_servidor(ip, puerto, logger, nombre_modulo);

    log_info(logger, "Servidor listo para recibir al cliente");

    int cliente_fd = esperar_cliente(server_fd, logger, nombre_modulo);

    return cliente_fd;
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


// conectarCon es para cuando sos un cliente y queres conectarte con el servidor.
int conectar_con(int modulo, t_config *config, t_log *logger){
    char* nombre_modulo;
    char* ip;
    char* puerto;
    int conexion;

    switch (modulo)
    {
    case KERNEL:
        ip = config_get_string_value(config, "IP_KERNEL");
        puerto = config_get_string_value(config, "PUERTO_KERNEL");
        nombre_modulo = strdup("Kernel");
        break;
    case CPU:
        ip = config_get_string_value(config, "IP_CPU");
        puerto = config_get_string_value(config, "PUERTO_CPU");
        nombre_modulo = strdup("CPU");
        break;
    case MEMORIA:
        ip = config_get_string_value(config, "IP_MEMORIA");
        puerto = config_get_string_value(config, "PUERTO_MEMORIA");
        nombre_modulo = strdup("Memoria");
        break;
    case FILESYSTEM:
        ip = config_get_string_value(config, "IP_FILESYSTEM");
        puerto = config_get_string_value(config, "PUERTO_FILESYSTEM");
        nombre_modulo = strdup("FileSystem");
        break;
    }
    
    log_info(logger, "El cliente se conectara a %s:%s", ip, puerto);
    
    if ((conexion = crear_conexion(logger, nombre_modulo, ip, puerto)) == 0){ // Si conexion = 0 significa que hubo error. Por eso detenemos la ejecución.
        // log_error(logger, "No se pudo establecer la conexión con el kernel."); // Medio al pepe este log porque ya crear_conexion tiene los log_error
        exit(2);
    }
    log_info(logger, "Conexión exitosa con el modulo %s", nombre_modulo); // No tengo idea para qué querría mostrar conexion pero bueno.

    return conexion;
}