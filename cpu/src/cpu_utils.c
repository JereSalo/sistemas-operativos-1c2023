#include "cpu_utils.h"

t_log* logger;
t_cpu_config config_cpu;
t_list* lista_parametros;
int cliente_kernel;


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


void ejecutar_proceso(t_contexto_ejecucion* contexto) {
    char* instruccion;
    char** instruccion_decodificada;

    while(!desalojado) {
        // Fetch: buscamos la proxima instruccion dada por el PC
        instruccion = list_get(contexto->instrucciones, contexto->pc);
        
        // Decode: interpretamos la instruccion (que intruccion es y que parametros lleva)
        instruccion_decodificada = string_split(instruccion, " "); // recordar que string_split hace que ult elemento sea NULL
        
        lista_parametros = list_create();
        
        ejecutar_instruccion(instruccion_decodificada, contexto);

        if (!desalojado) log_info(logger, "PID: %d - Instruccion %s finalizada \n", contexto->pid, instruccion);

        contexto->pc++;
    }

    // Si es desalojado =>
    desalojado = 0;

    log_info(logger, "PID: %d - Instruccion %s a ejecutar por parte del Kernel \n", contexto->pid, instruccion);

    send_contexto(cliente_kernel, contexto);
    send_desalojo(cliente_kernel, (intptr_t)dictionary_get(diccionario_instrucciones, instruccion_decodificada[0]), lista_parametros);

    list_destroy_and_destroy_elements(lista_parametros, free);
}


void ejecutar_instruccion(char** instruccion_decodificada, t_contexto_ejecucion* contexto) {
    char* nemonico_instruccion = instruccion_decodificada[0];  //PELADO BOTON QUE TE CREES DE LA RAE GIL, nemonico: palabra que sustituye a un codigo de operacion. pertenece a la memoria.

    log_warning(logger, "PID: %d - Ejecutando %s", contexto->pid, nemonico_instruccion); //logger obligatorio

    int op_instruccion = (intptr_t) dictionary_get(diccionario_instrucciones, nemonico_instruccion);

    switch(op_instruccion) {
        case SET:
        {
            // SET (Registro, Valor)

            char* registro = instruccion_decodificada[1];
            char* valor = instruccion_decodificada[2];
            
            usleep(config_cpu.RETARDO_INSTRUCCION * 1000);      // usleep trabaja con µs, hacemos *1000 para que sean ms
            
            asignar_a_registro(registro, valor, contexto->registros_cpu);

            // printf("EL REGISTRO %s QUEDO CON EL SIGUIENTE VALOR: %.*s \n", "AX", 4, contexto->registros_cpu->AX);
            // printf("VALORES DE TODOS LOS REGISTROS: %s \n", contexto->registros_cpu->AX);

            break;
        }
        case YIELD:
        case EXIT:
        {   
            desalojado = 1;
            break;        
        }
        case IO:
        case WAIT:
        case SIGNAL:
        {
            list_add(lista_parametros, instruccion_decodificada[1]);   

            desalojado = 1;
            
            break;   
        }
        default: {
            log_error(logger, "INSTRUCCION DESCONOCIDA");
        }
    }
}


void procesar_kernel() {
    while(1) {
        // Aca pensaba que había que usar semáforos pero no, el recv se encarga de recibir solo cuando el otro hace un send, sino se queda clavado.
        op_code cod_op = recibir_operacion(cliente_kernel);
        
        switch((int)cod_op) {
            case CONTEXTO_EJECUCION:
            {
                t_contexto_ejecucion* contexto = malloc(sizeof(t_contexto_ejecucion));
                
                if(!recv_contexto(cliente_kernel, contexto)) {
                    log_error(logger, "Fallo recibiendo CONTEXTO");
                    break;
                }

                ejecutar_proceso(contexto); // Se encarga también del desalojo del proceso, no hace falta poner nada abajo de esto

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