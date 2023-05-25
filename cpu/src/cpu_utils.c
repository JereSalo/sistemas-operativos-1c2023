#include "cpu_utils.h"

t_log* logger;
t_cpu_config config_cpu;
t_dictionary* diccionario_instrucciones;

int fin_proceso = 0;

void cargar_config_cpu(t_config* config) {
    
    config_cpu.RETARDO_INSTRUCCION = config_get_int_value(config, "RETARDO_INSTRUCCION");
    config_cpu.IP_MEMORIA          = config_get_string_value(config, "IP_MEMORIA");
    config_cpu.PUERTO_MEMORIA      = config_get_int_value(config, "PUERTO_MEMORIA");
    config_cpu.PUERTO_ESCUCHA      = config_get_int_value(config, "PUERTO_ESCUCHA");
    config_cpu.TAM_MAX_SEGMENTO    = config_get_int_value(config, "TAM_MAX_SEGMENTO");

    log_info(logger, "Config cargada en config_cpu");
}

void inicializar_diccionarios() {
    
    diccionario_instrucciones = dictionary_create();

    dictionary_put(diccionario_instrucciones, "SET", (int) SET); 
    dictionary_put(diccionario_instrucciones, "MOV_IN", (int) MOV_IN);
    dictionary_put(diccionario_instrucciones, "MOV_OUT", (int) MOV_OUT);
    dictionary_put(diccionario_instrucciones, "I_O", (int) I_O);
    dictionary_put(diccionario_instrucciones, "F_OPEN", (int) F_OPEN);
    dictionary_put(diccionario_instrucciones, "F_CLOSE", (int) F_CLOSE);
    dictionary_put(diccionario_instrucciones, "F_SEEK", (int) F_SEEK);
    dictionary_put(diccionario_instrucciones, "F_READ", (int)F_READ);
    dictionary_put(diccionario_instrucciones, "F_WRITE",(int) F_WRITE);
    dictionary_put(diccionario_instrucciones, "F_TRUNCATE",(int) F_TRUNCATE);
    dictionary_put(diccionario_instrucciones, "WAIT",(int) WAIT);
    dictionary_put(diccionario_instrucciones, "SIGNAL",(int) SIGNAL);
    dictionary_put(diccionario_instrucciones, "CREATE_SEGMENT",(int) CREATE_SEGMENT);
    dictionary_put(diccionario_instrucciones, "DELETE_SEGMENT",(int) DELETE_SEGMENT);
    dictionary_put(diccionario_instrucciones, "YIELD",(int) YIELD);
    dictionary_put(diccionario_instrucciones, "EXIT",(int) EXIT); 
}


void ejecutar_proceso(t_contexto_ejecucion* contexto) {

    char* instruccion;
    char** instruccion_decodificada;

    //mostrar_lista(contexto->instrucciones);

    while(!fin_proceso) {
    
        // Fetch: buscamos la proxima instruccion dada por el PC
        instruccion = list_get(contexto->instrucciones, contexto->pc);
        
        // Decode: interpretamos la instruccion (que intruccion es y que parametros lleva)
        instruccion_decodificada = string_split(instruccion, " "); // recordar que string_split hace que ult elemento sea NULL
        
        
        
        ejecutar_instruccion(instruccion_decodificada, contexto);

        // Cargamos los parametros de la instruccion en un string para mostrarlos en el logger prolijamente
        char parametros[100] = "";
        parametros_instruccion(instruccion_decodificada, parametros);


        log_info(logger, "PID: %d - Ejecutando %s %s", contexto->pid, instruccion_decodificada[0], parametros);

        contexto->pc++;

    }

    //guardar_contexto(contexto);        GUARDAR CONTEXO ACA

    if(fin_proceso) {               // Caso EXIT
        fin_proceso = 0;
        //HACER SEND DEL CONTEXTO AL KERNEL -> cuando termina el proceso debemos mandarlo
    }

    
}


void parametros_instruccion(char** instruccion_decodificada, char *parametros) {
    
    int tamanio_instruccion_decodificada = string_array_size(instruccion_decodificada);


    if (tamanio_instruccion_decodificada > 1) {
        strcat(parametros, "- ");  // Agrega el guion y el espacio si hay más de un elemento
    }

    for (int i = 1; i < tamanio_instruccion_decodificada; i++) {
        strcat(parametros, instruccion_decodificada[i]);  // Concatena el elemento actual

        if (i < tamanio_instruccion_decodificada - 1) {
            strcat(parametros, ", ");  // Agrega una coma y un espacio si no es el último elemento
        }
    }
}


void asignar_a_registro(char* registro, char* valor, t_contexto_ejecucion* contexto) {

    if(strcmp(registro, "AX") == 0) {
        strcpy(contexto->registros_cpu->AX, valor); //strcpy copia todo el string salvo el \0
    }

    

}


void ejecutar_instruccion(char** instruccion_decodificada, t_contexto_ejecucion* contexto) {

    // matcheamos con el primer elemento de la instruccion decodificada, osea la FIRMA de la instruccion
    // esto nos tira un warning por el tema del casteo -> revisar!! (Me parece que con intptr_t se soluciona)

    int op_instruccion = (intptr_t) dictionary_get(diccionario_instrucciones, instruccion_decodificada[0]);

    switch(op_instruccion) {
        case SET:
        {
            printf("EJECUTE SET \n");

            //aca faltaria hacer el sleep del retardo del archivo de config del cpu
            //para eso deberiamos cargar en un struct todo lo del archivo de config como hicimos con el kernel
            usleep(config_cpu.RETARDO_INSTRUCCION * 1000);
            
            asignar_a_registro(instruccion_decodificada[1], instruccion_decodificada[2], contexto);

            printf("EL REGISTRO %s QUEDO CON EL SIGUIENTE VALOR: %s \n", instruccion_decodificada[1], contexto->registros_cpu->AX);


            break;
        }
        case YIELD:
        {
            printf("EJECUTE YIELD \n");  
            break;        
        }
        case EXIT:
        {
            printf("EJECUTE EXIT \n");
            fin_proceso = 1;
            //HAY QUE DEVOLVER EL CONTEXTO DE EJECUCION AL KERNEL Y ADEMAS EL KERNEL TIENE QUE ELIMINAR EL PCB
            // Y DISMINUIR EN UNA UNIDAD EL SEMAFORO DE GRADO DE MULTIPROGRAMACION -> esto lo hacemos en el while
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

                ejecutar_proceso(contexto);

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