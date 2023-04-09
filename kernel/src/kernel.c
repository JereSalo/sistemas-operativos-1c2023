#include "kernel.h"

int main(int argc, char** argv){
    // Somos el server y queremos que el cliente se conecte con nosotros.
    // Vamos a iniciar el servidor para poder escuchar al cliente y esperaremos su conexión

    t_log *logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    t_config *config = config_create("kernel.config"); // Aca meti la IP y el Puerto del kernel, que se repite ya en consola, no es buena práctica pero es sencillo.

    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");


	int server_fd = iniciar_servidor(ip_kernel, puerto_kernel, logger, "Kernel");

	log_info(logger, "Servidor listo para recibir al cliente");

	int cliente_fd = esperar_cliente(server_fd, logger, "Kernel");
}

