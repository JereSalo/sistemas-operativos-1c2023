#include "cpu.h"

int main(int argc, char** argv){
    
    int modulo = CPU;
    t_log *logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);
    t_config *config = config_create("cpu.config");


    // CLIENTE -> Memoria
    int conexion = conectar_con(MEMORIA, config, logger);

    // SERVER -> Kernel
    int cliente_fd = iniciar_servidor_y_esperar_cliente(modulo, config, logger);
}