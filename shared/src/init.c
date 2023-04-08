#include "init.h"

bool generar_conexiones(t_log* logger, int* socket_kernel) {
    //LEVANTAR DEL ARCHIVO DE CONFIG IP Y PUERTO

    t_config* config = config_create("consola.config");

    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    printf(ip_kernel,"%s");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");

    *socket_kernel = crear_conexion(logger, "KERNEL", ip_kernel, puerto_kernel);

    return *socket_kernel != 0;
}

void cerrar_programa(t_log* logger) {
    log_destroy(logger);
}