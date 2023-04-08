#include "consola.h"

// SOMOS EL CLIENTE

int main(int argc, char** argv){

    t_log *logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

    /*
    int socket_kernel = 0;

    if(!generar_conexiones(logger, &socket_kernel)) {
        cerrar_programa(logger);
    }
    */
 
   // el modulo es un codModulo
    int conexion = conectarseCon(modulo, logger); // Esta función devuelve el socket con la conexion ya hecha.
    if (conexion == 0){ /*hubo un error*/}
    

    //SI ESTA TODO BIEN PODEMOS MANDAR MENSAJES
    
}

/*
// modulo es un codModulo
tipoRaro conectarseCon(modulo, logger){
    ip = obtenerIp(modulo);
    puerto = obtenerPuerto(modulo);
    nombreMoudlo = obtenerNombre(modulo);

    return crear_conexion(logger,nombreModulo,ip,puerto);
}

obtenerIp(modulo){
  
    Switch por modulo
  

    return config_get_string_value(config, "IP_KERNEL");
}
*/
