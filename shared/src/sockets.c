#include "sockets.h"

/* ----------------------------------- SERVIDOR ----------------------------------- */

// INICIA SERVER ESCUCHANDO EN IP:PUERTO
int iniciar_servidor(char *ip, char *puerto, t_log *logger, const char *name)
{

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

    // Estas 2 lineas solucionan problema de cuando finalizas un servidor con Ctrl C antes de "matar" a sus clientes.
    // Fuente: https://github.com/sisoputnfrba/foro/issues/2804 ;D
    int yes = 1;
    setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    // Se asocia el socket a un puerto
    bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen);

    // Se escuchan las conexiones entrantes (hasta SOMAXCONN conexiones simultaneas)
    listen(socket_servidor, SOMAXCONN);

    log_info(logger, "Escuchando en %s:%s (%s)\n", ip, puerto, name);

    freeaddrinfo(server_info);

    return socket_servidor;
}

// ESPERAR CONEXION DE CLIENTE EN UN SERVER ABIERTO
int esperar_cliente(int socket_servidor, t_log *logger, const char *name)
{

    // Aceptamos un nuevo cliente
    log_debug(logger, "Esperando a un cliente\n");
    int socket_cliente = accept(socket_servidor, NULL, NULL);

    if (socket_cliente == -1)
    {
        log_error(logger, "Fallo del %s al aceptar la conexión entrante \n", name); // Capaz el log podria ser mejor
        return -1;                                                               // Ya se que es lo mismo que dejar return socket_cliente porque vale -1 pero me parece mejor dejarlo claro de esta forma.
    }

    log_debug(logger, "Cliente conectado (a %s)\n", name);

    return socket_cliente;
}




int recibir_operacion(int socket_cliente)
{
    int cod_op;
    // Lo clave de esto es que recv es bloqueante, si no hay nada para recibir entonces se queda clavado.
    // Si el cliente muere entonces retorna -1 (error) y así después en el else cierro la conexión.
    if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
        return cod_op;
    else
    {
        close(socket_cliente);
        return -1;
    }
}




// Hace lo que dice. Inicia el servidor, espera que el cliente se conecte. Cuando se conecta devuelve el socket con la conexión.
int preparar_servidor(char* nombre_modulo, t_config *config, t_log *logger)
{
    bool local = false; // Podriamos crear un config aparte que sea solo para nosotros en el que pongamos este tipo de cosas, asi no tocamos codigo. Podriamos hacer uno para activar/desactivar los loggers por ejemplo.

    char* ip;
    if(local)
        ip = obtener_ip_local();
    else
        ip = obtener_ip_red();
    
    char *puerto = config_get_string_value(config, "PUERTO_ESCUCHA");;
    
    int server_fd = iniciar_servidor(ip, puerto, logger, nombre_modulo);

    log_info(logger, "Servidor listo para recibir al cliente \n");

    free(ip);

    return server_fd;
}


/* ----------------------------------- CLIENTE ----------------------------------- */

// CLIENTE SE INTENTA CONECTAR A SERVER ESCUCHANDO EN IP:PUERTO
int crear_conexion(t_log *logger, const char *server_name, char *ip, char *puerto)
{

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
    if (socket_cliente == -1)
    {
        log_error(logger, "Error creando el socket para %s:%s \n", ip, puerto);
        return 0;
    }

    // Error conectando
    // connect REALIZA la conexión y además retorna -1 si salió mal, si salió bien retorna un 0.
    if (connect(socket_cliente, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        log_error(logger, "Error al conectar (a %s)\n", server_name);
        freeaddrinfo(servinfo);
        return 0;
    }
    else
        log_info(logger, "Cliente conectado en %s:%s (a %s)\n", ip, puerto, server_name);

    freeaddrinfo(servinfo);

    return socket_cliente;
}

// CERRAR CONEXION
void liberar_conexion(int *socket_cliente)
{
    close(*socket_cliente);
    *socket_cliente = -1;
}

void cerrar_programa(t_log *logger, t_config *config)
{
    log_destroy(logger);
    config_destroy(config);
    
}

// conectarCon es para cuando sos un cliente y queres conectarte con el servidor.
int conectar_con(int modulo, t_config *config, t_log *logger)
{
    char *nombre_modulo;
    char *ip;
    char *puerto;
    int conexion;

    switch (modulo)
    {
    case KERNEL:
        ip = config_get_string_value(config, "IP_KERNEL");
        puerto = config_get_string_value(config, "PUERTO_KERNEL");
        nombre_modulo = "KERNEL";
        break;
    case CPU:
        ip = config_get_string_value(config, "IP_CPU");
        puerto = config_get_string_value(config, "PUERTO_CPU");
        nombre_modulo = "CPU";
        break;
    case MEMORIA:
        ip = config_get_string_value(config, "IP_MEMORIA");
        puerto = config_get_string_value(config, "PUERTO_MEMORIA");
        nombre_modulo = "MEMORIA";
        break;
    case FILESYSTEM:
        ip = config_get_string_value(config, "IP_FILESYSTEM");
        puerto = config_get_string_value(config, "PUERTO_FILESYSTEM");
        nombre_modulo = "FILESYSTEM";
        break;
    }

    log_info(logger, "El cliente se conectara a %s:%s \n", ip, puerto);

    if ((conexion = crear_conexion(logger, nombre_modulo, ip, puerto)) == 0)
    { // Si conexion = 0 significa que hubo error. Por eso detenemos la ejecución.
        // log_error(logger, "No se pudo establecer la conexión con el kernel."); // Medio al pepe este log porque ya crear_conexion tiene los log_error
        exit(2);
    }
    log_info(logger, "Conexión exitosa con el modulo %s \n", nombre_modulo); // No tengo idea para qué querría mostrar conexion pero bueno.

    return conexion;
}


// FUNCIONES PARA OBTENER IP LOCAL Y DE RED

char* obtener_ip_red(){
    int sockfd;
    struct sockaddr_in servaddr;
    
    // Crear un socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    // Configurar la dirección del servidor
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(80);
    if (inet_pton(AF_INET, "8.8.8.8", &(servaddr.sin_addr)) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
    
    // Conectarse al servidor (se utiliza el servidor DNS de Google como ejemplo)
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    // Obtener la dirección local asociada al socket
    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    if (getsockname(sockfd, (struct sockaddr*)&localaddr, &addrlen) < 0) {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }
    
    // Convertir la dirección IP a un formato legible por humanos
    char ip_address[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(localaddr.sin_addr), ip_address, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop");
        exit(EXIT_FAILURE);
    }
    
    // Cerrar el socket
    close(sockfd);

    return strdup(ip_address);
}

char* obtener_ip_local(){
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Error al crear el socket");
    }

    // Conectarse a una dirección de loopback y un puerto arbitrario
    struct sockaddr_in loopback;
    memset(&loopback, 0, sizeof(loopback));
    loopback.sin_family = AF_INET;
    loopback.sin_addr.s_addr = inet_addr("127.0.0.2");
    loopback.sin_port = htons(12345);

    if (connect(sock, (struct sockaddr*)&loopback, sizeof(loopback)) < 0) {
        perror("Error al conectarse al socket de loopback");
    }

    // Obtener la dirección IP de la conexión establecida
    struct sockaddr_in localAddr;
    socklen_t addrLen = sizeof(localAddr);
    if (getsockname(sock, (struct sockaddr*)&localAddr, &addrLen) < 0) {
        perror("Error al obtener la dirección IP");
        close(sock);
    }

    // Convertir la dirección IP de binario a una cadena legible
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(localAddr.sin_addr), ipStr, INET_ADDRSTRLEN);

    // Cerrar el socket
    close(sock);

    return strdup(ipStr);
}
