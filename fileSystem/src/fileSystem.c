#include "../include/fileSystem.h"

int main(int argc, char** argv){
    int modulo = FILESYSTEM;
    
    t_log *logger = log_create("filesystem.log", "FILESYSTEM", true, LOG_LEVEL_INFO);
    t_config *config = config_create("filesystem.config");
    
    // CLIENTE -> Memoria
    //int conexion = conectar_con(MEMORIA, config, logger);


    // SERVER
    int cliente_fd = iniciar_servidor_y_esperar_cliente(modulo, config, logger);

}