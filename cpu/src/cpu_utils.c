#include "cpu_utils.h"

t_log* logger;
t_dictionary* diccionario_instrucciones;


void ejecutar_proceso(t_contexto_ejecucion* contexto) {

    char* instruccion;
    char** instruccion_decodificada;

    mostrar_lista(contexto->instrucciones);
/*
    //while(contexto->pc < list_size(contexto->instrucciones)) {
    
        // Fetch: buscamos la proxima instruccion dada por el PC
        instruccion = list_get(contexto->instrucciones, contexto->pc);
        
        // Decode: interpretamos la instruccion (que intruccion es y que parametros lleva)
        instruccion_decodificada = string_split(instruccion, " ");

        //["SET", "AX", "HOLA"]
        
        printf("PROGRAM COUNTER STRIKE %d", contexto->pc);
        ejecutar_instruccion(instruccion_decodificada, contexto);
        
        //contexto->pc++;

    }
    */
}


void ejecutar_instruccion(char** instruccion_decodificada, t_contexto_ejecucion* contexto) {

    // matcheamos con el primer elemento de la instruccion decodificada, osea la FIRMA de la instruccion
    int op_instruccion = (int) dictionary_get(diccionario_instrucciones, instruccion_decodificada[0]);

    switch(op_instruccion) {
        case SET:
        {
            printf("EJECUTE SET \n");
            break;
        }
        case EXIT:
        {
            printf("EJECUTE EXIT \n");
            break;   
        }
        case YIELD:
        {
            printf("EJECUTE YIELD \n");  
            break;        
        }
        default: {
            printf("INSTRUCCION DESCONOCIDA \n");
        }
    }
}


void procesar_conexion_cpu(int cliente_socket) {
    while(1) {
        // Aca pensaba que había que usar semáforos pero no, el recv se encarga de recibir solo cuando el otro hace un send, sino se queda clavado.
        op_code cod_op = recibir_operacion(cliente_socket);
        
        switch((int)cod_op) {
            case CONTEXTO_EJECUCION:
            {
                printf("El cop que me llegó es Contexto Ejecucion\n");
                t_contexto_ejecucion* contexto = malloc(sizeof(t_contexto_ejecucion));
                contexto->registros_cpu = malloc(sizeof(t_registros_cpu));
                contexto->instrucciones = list_create();
                
                if(!recv_contexto(cliente_socket, contexto)) {
                    log_error(logger, "Fallo recibiendo CONTEXTO");
                    break;
                }

                log_info(logger, "RECIBI BIEN EL CONTEXTO");

                log_info(logger, "Contexto PID: %d", contexto->pid);
                log_info(logger, "Contexto PC: %d", contexto->pc);
                //log_info(logger, "REGISTRO AX en posicion 2: %d", contexto->registros_cpu->AX[2]);

                ejecutar_proceso(contexto);


                // Falta tema registros. No va a poder mostrar la lista bien hasta que los registros no esten hechos (por el orden de serializacion)
                // mostrar_lista(contexto->instrucciones);

                break;
            }

            case -1:
            {
			    log_error(logger, "El cliente se desconecto. Terminando Servidor");
			    return;
            }
		    default:
            {
			    log_warning(logger,"Operación desconocida. Hubo un problemita !");
			    break;
            }
        }
    }
}