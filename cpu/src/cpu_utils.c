#include "cpu_utils.h"

t_log* logger;
t_cpu_config config_cpu;



int fin_proceso = 0;

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

    //mostrar_lista(contexto->instrucciones);

    while(!fin_proceso) {
    
        // Fetch: buscamos la proxima instruccion dada por el PC
        instruccion = list_get(contexto->instrucciones, contexto->pc);
        
        // Decode: interpretamos la instruccion (que intruccion es y que parametros lleva)
        instruccion_decodificada = string_split(instruccion, " "); // recordar que string_split hace que ult elemento sea NULL
        
        
        ejecutar_instruccion(instruccion_decodificada, contexto);

        log_info(logger, "PID: %d - Ejecutando %s", contexto->pid, instruccion); //logger obligatorio

        contexto->pc++;

    }

    if(fin_proceso) {               // Caso EXIT
        fin_proceso = 0;
        
        
        //char* motivo_desalojo = strdup("DESALOJO POR EXIT");
        //contexto->motivo_desalojo = instruccion;

        //cliente socket es kernel
        printf("ESTOY POR ENTRAR A SEND");
        
        
        //ACA ESTAMOS TENIENDO PROBLEMAS CON EL ENVIO DEL CONTEXTO AL KERNEL !!!!!!!
        
        // send_contexto(cliente_socket, contexto);
        //send_string(cliente_socket, motivo_desalojo); 
        
        
        
        //HACER SEND DEL CONTEXTO AL KERNEL -> cuando termina el proceso debemos mandarlo
    }

    
}


void ejecutar_instruccion(char** instruccion_decodificada, t_contexto_ejecucion* contexto) {
    char* nemonico_instruccion = instruccion_decodificada[0]; 

    int op_instruccion = (intptr_t) dictionary_get(diccionario_instrucciones, nemonico_instruccion);

    switch(op_instruccion) {
        case SET:
        {
            // SET (Registro, Valor)
            printf("EJECUTANDO SET \n");

            char* registro = instruccion_decodificada[1];
            char* valor = instruccion_decodificada[2];

            usleep(config_cpu.RETARDO_INSTRUCCION * 1000);      // usleep trabaja con µs, hacemos *1000 para que sean ms
            
            asignar_a_registro(registro, valor, contexto->registros_cpu);

            printf("EL REGISTRO %s QUEDO CON EL SIGUIENTE VALOR: %.*s \n", "AX", 4, contexto->registros_cpu->AX);
            printf("VALORES DE TODOS LOS REGISTROS: %s \n", contexto->registros_cpu->AX);

            break;
        }
        case YIELD:
        {
            // YIELD
            printf("EJECUTE YIELD \n");  
            break;        
        }
        case EXIT:
        {
            // EXIT
            printf("EJECUTE EXIT \n");
            fin_proceso = 1;
            //HAY QUE DEVOLVER EL CONTEXTO DE EJECUCION AL KERNEL Y ADEMAS EL KERNEL TIENE QUE ELIMINAR EL PCB
            // Y DISMINUIR EN UNA UNIDAD EL SEMAFORO DE GRADO DE MULTIPROGRAMACION -> esto lo hacemos en el while
            
            //contexto->motivo_fin_proceso = SUCCESS;
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
                log_info(logger, "El cop que me llegó es Contexto Ejecucion");
                t_contexto_ejecucion* contexto = malloc(sizeof(t_contexto_ejecucion));
                contexto->registros_cpu = malloc(sizeof(t_registros_cpu));
                contexto->instrucciones = list_create(); // Puse esto aca porque list_create() es como un malloc
                
                if(!recv_contexto(cliente_socket, contexto)) {
                    log_error(logger, "Fallo recibiendo CONTEXTO");
                    break;
                }

                log_info(logger, "RECIBI BIEN EL CONTEXTO");

                log_info(logger, "Contexto PID: %d", contexto->pid);
                // log_info(logger, "Contexto PC: %d", contexto->pc);
                
                // mostrar_lista(contexto->instrucciones);

           
                //log_info(logger, "REGISTRO AX en posicion 2: %d", contexto->registros_cpu->AX[2]);

                ejecutar_proceso(contexto, cliente_socket);

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