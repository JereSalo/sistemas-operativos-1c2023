#include "../include/kernel.h"

int main(int argc, char** argv){
    t_log *logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    t_config *config = config_create("kernel.config");
    int modulo = KERNEL;

    int cliente_fd = iniciar_servidor_y_esperar_cliente(modulo, config, logger);

    int conexion = conectar_con(CPU, config, logger);

}


