#include "cpu_utils.h"

t_log* logger;
t_cpu_config config_cpu;



int desalojado = 0;
// void* paquete_desalojo;
// size_t tamanio_paquete_desalojo;

void cargar_config_cpu(t_config* config) {
    
    config_cpu.RETARDO_INSTRUCCION = config_get_int_value(config, "RETARDO_INSTRUCCION");
    config_cpu.IP_MEMORIA          = config_get_string_value(config, "IP_MEMORIA");
    config_cpu.PUERTO_MEMORIA      = config_get_int_value(config, "PUERTO_MEMORIA");
    config_cpu.PUERTO_ESCUCHA      = config_get_int_value(config, "PUERTO_ESCUCHA");
    config_cpu.TAM_MAX_SEGMENTO    = config_get_int_value(config, "TAM_MAX_SEGMENTO");

    log_info(logger, "Config cargada en config_cpu");
}





void ejecutar_proceso(t_contexto_ejecucion* contexto, int cliente_socket) {
    char* instruccion;
    char** instruccion_decodificada;

    while(!desalojado) {
        // Fetch: buscamos la proxima instruccion dada por el PC
        instruccion = list_get(contexto->instrucciones, contexto->pc);
        
        // Decode: interpretamos la instruccion (que intruccion es y que parametros lleva)
        instruccion_decodificada = string_split(instruccion, " "); // recordar que string_split hace que ult elemento sea NULL
        
        ejecutar_instruccion(instruccion_decodificada, contexto);

        log_info(logger, "Instruccion %s finalizada", instruccion);

        contexto->pc++;
    }

    if(desalojado) {               // Caso instrucción con desalojo
        desalojado = 0;
        
        
        //char* motivo_desalojo = strdup("DESALOJO POR EXIT");
        //contexto->motivo_desalojo = instruccion;

        //cliente socket es kernel
        printf("ESTOY POR ENTRAR A SEND");
        

        send_contexto(cliente_socket, contexto); // Esto me parece bien hacerlo aca
        
        // send(cliente_socket, instruccion, strlen(instruccion) + 1, 0); // No esta chequeado esto

        // send(cliente_socket, paquete_desalojo, tamanio_paquete_desalojo, 0);
        // No se que opinan sobre armar paquete_desalojo dentro del switch de ejecutar_instruccion y poner su tamaño ahí. Onda que la serialización ocurra ahí adentro, porque depende de cada funcion, pero no se.


        //send_string(cliente_socket, motivo_desalojo); 
        
        
        
        
        //HACER SEND DEL CONTEXTO AL KERNEL -> cuando termina el proceso debemos mandarlo
    }

    
}


void ejecutar_instruccion(char** instruccion_decodificada, t_contexto_ejecucion* contexto) {
    char* nemonico_instruccion = instruccion_decodificada[0];  //PELADO BOTON QUE TE CREES DE LA RAE GIL, nemonico: palabra que sustituye a un codigo de operacion. pertenece a la memoria.

    log_warning(logger, "PID: %d - Ejecutando %s", contexto->pid, nemonico_instruccion); //logger obligatorio

    int op_instruccion = (intptr_t) dictionary_get(diccionario_instrucciones, nemonico_instruccion);

    switch(op_instruccion) {
        case SET:
        {
            // SET (Registro, Valor)
            log_info(logger, "EJECUTANDO SET");

            char* registro = instruccion_decodificada[1];
            char* valor = instruccion_decodificada[2];

            usleep(config_cpu.RETARDO_INSTRUCCION * 1000);      // usleep trabaja con µs, hacemos *1000 para que sean ms
            
            asignar_a_registro(registro, valor, contexto->registros_cpu);

            // printf("EL REGISTRO %s QUEDO CON EL SIGUIENTE VALOR: %.*s \n", "AX", 4, contexto->registros_cpu->AX);
            // printf("VALORES DE TODOS LOS REGISTROS: %s \n", contexto->registros_cpu->AX);

            break;
        }
        case YIELD:
        {
            // YIELD
            printf("EJECUTE YIELD \n");
            desalojado = 1;
            break;        
        }
        case EXIT:
        {
            // EXIT
            printf("EJECUTE EXIT \n");
            desalojado = 1;
            //HAY QUE DEVOLVER EL CONTEXTO DE EJECUCION AL KERNEL Y ADEMAS EL KERNEL TIENE QUE ELIMINAR EL PCB
            // Y DISMINUIR EN UNA UNIDAD EL SEMAFORO DE GRADO DE MULTIPROGRAMACION -> esto lo hacemos en el while
            
            //contexto->motivo_desalojado = SUCCESS;
            break;   
        }
        default: {
            log_error(logger, "INSTRUCCION DESCONOCIDA");
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
                log_info(logger, "El cop que me llegó es Contexto Ejecucion");
                t_contexto_ejecucion* contexto = malloc(sizeof(t_contexto_ejecucion));
                
                if(!recv_contexto(cliente_socket, contexto)) {
                    log_error(logger, "Fallo recibiendo CONTEXTO");
                    break;
                }

                log_info(logger, "Recibi el Contexto del Proceso %d",contexto->pid);

                ejecutar_proceso(contexto, cliente_socket); // esta funcion podria modificar el contexto y retornar el motivo con los parametros?

                // send_contexto(contexto, cliente_socket);
                // send motivo_con_parametros(motivo, cliente_socket);



                break;
            }

            case -1:
            {
			    log_error(logger, "El cliente se desconecto. Terminando Servidor \n");
			    return;
            }
		    default:
            {
			    log_error(logger,"Operación desconocida. Hubo un problemita! \n");
			    break;
            }
        }
    }
}