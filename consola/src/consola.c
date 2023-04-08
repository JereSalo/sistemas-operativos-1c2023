#include "consola.h"

int main(int argc, char** argv){

    t_log *logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

    int socket_kernel = 0;

    if(!generar_conexiones(logger, &socket_kernel)) {
        cerrar_programa(logger);
    }

    //SI ESTA TODO BIEN PODEMOS MANDAR MENSAJES


    
}