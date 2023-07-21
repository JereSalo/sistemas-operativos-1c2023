#include "procesar_filesystem_memoria.h"

void procesar_filesystem_memoria(){
    while(1) {
        op_code cod_op = recibir_operacion(cliente_filesystem);

        switch((int)cod_op){
            case SOLICITUD_ESCRITURA: // cambiar nombre ?? XD
            {
                int direccion_fisica;
                int longitud;
                RECV_INT(cliente_filesystem, direccion_fisica);
                RECV_INT(cliente_filesystem, longitud);
                //Falta RECV del PID?


                char* valor_a_escribir = (char*)(recv_paquete(cliente_filesystem, (size_t)longitud));

                usleep(config_memoria.RETARDO_MEMORIA * 1000); // Acceso a espacio de usuario
                memcpy(memoria_principal + direccion_fisica, valor_a_escribir, longitud);
                

                //printf("DATOS ESCRITOS EN MEMORIA: %s", valor_a_escribir);
                
                free(valor_a_escribir);

                //Debug
                //leer_memoria();

                send_string(cliente_filesystem, "OK");

                break;
            }
        }
    }
}