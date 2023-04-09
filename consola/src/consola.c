#include "consola.h"

// SOMOS EL CLIENTE

int main(int argc, char** argv){
    // Consola va a recibir como parámetro del main 2 path:
    //  1: Archivo de configuración. consola.config (Por ahora lo dejamos en la carpeta consola, pero se manda por parámetro la ruta así que puede ser cualquiera)
    //  2: Archivo de pseudocódigo con las instrucciones a ejecutar.

    /*
    1:
    En el archivo de configuracion vamos a tener la IP y el Puerto del Kernel.
    Con esos datos ya basta para establecer la conexión con el mismo.
    crear_conexion(logger, nombre, ip, puerto)
    En nuestro caso sería
    crear_conexion(logger, "Kernel", kernelIP, kernelPuerto);
    Esto lo guardamos en una variable ya que si salió todo bien devuelve un socket. Caso contrario, devuelve un 0.
    */

    /*
    2:
    Una vez que la conexión con el kernel ya está establecida la idea es mandarle las instrucciones del archivo (2)
    Para esto tendríamos que preparar un "paquete" con las instrucciones y una vez terminado, enviarlo.
       - Crear paquete: Tenemos que parsear cada línea terminada con "/n" como una instrucción y generar el listado de instrucciones.
    Luego de haberlo enviado la consola quedará a la espera de un mensaje del Kernel que indique que finalizó el proceso.
    */

    t_log *logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);
    t_config *config = config_create("consola.config"); // Tiene IP y Puerto. En realidad cuando se mande por parámetro del main la ruta va a ser diferente, por ahora es así.
    int conexion; // Guarda la conexión con el servidor.

    char* ipKernel = config_get_string_value(config, "IP_KERNEL");
    char* puertoKernel = config_get_string_value(config, "PUERTO_KERNEL");
    
    log_info(logger, "El cliente se conectara a %s:%s", ipKernel, puertoKernel);
    
    if ((conexion = crear_conexion(logger, "Kernel", ipKernel, puertoKernel)) == 0){ // Si conexion = 0 significa que hubo error. Por eso detenemos la ejecución.
        // log_error(logger, "No se pudo establecer la conexión con el kernel."); // Medio al pepe este log porque ya crear_conexion tiene los log_error
        exit(2);
    }
    log_info(logger, "Conexión exitosa con el kernel: %d", conexion); // No tengo idea para qué querría mostrar conexion pero bueno.


    //SI ESTA TODO BIEN PODEMOS MANDAR MENSAJES
}