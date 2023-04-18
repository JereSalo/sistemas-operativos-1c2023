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
    t_config *config = config_create("consola.config"); 
    // Config Tiene IP y Puerto. En realidad cuando se mande por parámetro del main la ruta va a ser diferente, por ahora es así.

    
    // CLIENTE -> Kernel

    // Conexion con Kernel
    int conexion = conectar_con(KERNEL, config, logger);


    
    // ENVIO DE MENSAJES

    int numero;
    printf("Ingrese un numero: ");
    scanf("%d", &numero);
    send_numero(conexion, numero);

    config_destroy(config);
    log_destroy(logger);
}


