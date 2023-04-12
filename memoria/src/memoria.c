#include "memoria.h"

int main(int argc, char** argv){
    int modulo = MEMORIA;

    t_log *logger = log_create("memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
    t_config *config = config_create("memoria.config");

    
    // SERVER -> CPU, Kernel, FileSystem
    int cliente_fd = iniciar_servidor_y_esperar_cliente(modulo, config, logger);
}