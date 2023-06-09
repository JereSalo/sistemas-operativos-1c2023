#include "file_system_utils.h"

t_log* logger;
t_config* config;
t_filesystem_config config_filesystem;
int cliente_kernel;
int server_memoria;

void cargar_config_filesystem(t_config* config){
    config_filesystem.IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    config_filesystem.PUERTO_MEMORIA = config_get_int_value(config, "PUERTO_MEMORIA");
    config_filesystem.PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
    config_filesystem.PATH_SUPERBLOQUE = config_get_string_value(config, "PATH_SUPERBLOQUE");
    config_filesystem.PATH_BITMAP = config_get_string_value(config, "PATH_BITMAP");
    config_filesystem.PATH_BLOQUES = config_get_string_value(config, "PATH_BLOQUES");
    config_filesystem.PATH_FCB = config_get_string_value(config, "PATH_FCB");
    config_filesystem.RETARDO_ACCESO_BLOQUE = config_get_int_value(config, "RETARDO_ACCESO_BLOQUE");

    log_info(logger, "Config cargada en filesystem");
}
