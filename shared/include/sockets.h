#ifndef SOCKETS_H_
#define SOCKETS_H_
#define _GNU_SOURCE

#include "protocolo.h"
#include "shared_utils.h"


int iniciar_servidor(char*, char*, t_log*, const char*);
int esperar_cliente(int, t_log*, const char*);
int crear_conexion(t_log* , const char* , char* , char*);
int recibir_operacion(int);
void liberar_conexion(int* socket_cliente);
int conectar_con(int, t_config *, t_log*);
int preparar_servidor(char* nombre_modulo, t_config *config, t_log *logger);
void esperar_clientes(int server_socket, t_log* logger, char* nombre_server);
void cerrar_programa(t_log* logger, t_config* config);

char* obtener_ip_red();
char* obtener_ip_local();

typedef enum { // Los estados que puede tener un PCB
    KERNEL,
    CPU,
    FILESYSTEM,
    MEMORIA
} cod_modulo;

#endif


