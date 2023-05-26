#include "cpu_utils.h"

t_log* logger;
t_cpu_config config_cpu;
t_dictionary* diccionario_instrucciones;
t_dictionary* diccionario_registros_cpu;


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
    
    // Diccionario de instrucciones
    diccionario_instrucciones = dictionary_create();

    dictionary_put(diccionario_instrucciones, "SET", (void*) (intptr_t) SET); 
    dictionary_put(diccionario_instrucciones, "MOV_IN", (void*) (intptr_t) MOV_IN);
    dictionary_put(diccionario_instrucciones, "MOV_OUT", (void*) (intptr_t) MOV_OUT);
    dictionary_put(diccionario_instrucciones, "I_O", (void*) (intptr_t) I_O);
    dictionary_put(diccionario_instrucciones, "F_OPEN", (void*) (intptr_t) F_OPEN);
    dictionary_put(diccionario_instrucciones, "F_CLOSE", (void*) (intptr_t) F_CLOSE);
    dictionary_put(diccionario_instrucciones, "F_SEEK", (void*) (intptr_t) F_SEEK);
    dictionary_put(diccionario_instrucciones, "F_READ", (void*) (intptr_t) F_READ);
    dictionary_put(diccionario_instrucciones, "F_WRITE",(void*) (intptr_t) F_WRITE);
    dictionary_put(diccionario_instrucciones, "F_TRUNCATE",(void*) (intptr_t) F_TRUNCATE);
    dictionary_put(diccionario_instrucciones, "WAIT",(void*) (intptr_t) WAIT);
    dictionary_put(diccionario_instrucciones, "SIGNAL",(void*) (intptr_t) SIGNAL);
    dictionary_put(diccionario_instrucciones, "CREATE_SEGMENT",(void*) (intptr_t) CREATE_SEGMENT);
    dictionary_put(diccionario_instrucciones, "DELETE_SEGMENT",(void*) (intptr_t) DELETE_SEGMENT);
    dictionary_put(diccionario_instrucciones, "YIELD",(void*) (intptr_t) YIELD);
    dictionary_put(diccionario_instrucciones, "EXIT",(void*) (intptr_t) EXIT); 

    // Diccionario de registros CPU
    diccionario_registros_cpu = dictionary_create();

    dictionary_put(diccionario_registros_cpu, "AX",  (void*) (intptr_t) AX);
    dictionary_put(diccionario_registros_cpu, "BX",  (void*) (intptr_t) BX);
    dictionary_put(diccionario_registros_cpu, "CX",  (void*) (intptr_t) CX);
    dictionary_put(diccionario_registros_cpu, "DX",  (void*) (intptr_t) DX);
    dictionary_put(diccionario_registros_cpu, "EAX",  (void*) (intptr_t) EAX);
    dictionary_put(diccionario_registros_cpu, "EBX",  (void*) (intptr_t) EBX);
    dictionary_put(diccionario_registros_cpu, "ECX",  (void*) (intptr_t) ECX);
    dictionary_put(diccionario_registros_cpu, "EDX",  (void*) (intptr_t) EDX);
    dictionary_put(diccionario_registros_cpu, "RAX",  (void*) (intptr_t) RAX);
    dictionary_put(diccionario_registros_cpu, "RBX",  (void*) (intptr_t) RBX);
    dictionary_put(diccionario_registros_cpu, "RCX",  (void*) (intptr_t) RCX);
    dictionary_put(diccionario_registros_cpu, "RDX",  (void*) (intptr_t) RDX);
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

void asignar_a_registro(char* registro, char* valor, t_contexto_ejecucion* contexto) {

    registro_cpu reg = (intptr_t) dictionary_get(diccionario_registros_cpu, registro);

    //strcpy copia todo el string salvo el \0
    
    switch(reg) {
        case AX: 
            strcpy(contexto->registros_cpu->AX, valor); 
            break;
        case BX: 
            strcpy(contexto->registros_cpu->BX, valor); 
            break;
        case CX: 
            strcpy(contexto->registros_cpu->CX, valor); 
            break;
        case DX: 
            strcpy(contexto->registros_cpu->DX, valor); 
            break;
        case EAX: 
            strcpy(contexto->registros_cpu->EAX, valor); 
            break;
        case EBX: 
            strcpy(contexto->registros_cpu->EBX, valor); 
            break;
        case ECX: 
            strcpy(contexto->registros_cpu->ECX, valor); 
            break;
        case EDX: 
            strcpy(contexto->registros_cpu->EDX, valor); 
            break;
        case RAX: 
            strcpy(contexto->registros_cpu->RAX, valor); 
            break;
        case RBX: 
            strcpy(contexto->registros_cpu->RBX, valor); 
            break;
        case RCX: 
            strcpy(contexto->registros_cpu->RCX, valor); 
            break;
        case RDX: 
            strcpy(contexto->registros_cpu->RDX, valor); 
            break;
        default:
            printf("ERROR: EL REGISTRO NO EXISTE !!! \n");
    }
}


void ejecutar_instruccion(char** instruccion_decodificada, t_contexto_ejecucion* contexto) {

    // matcheamos con el primer elemento de la instruccion decodificada, osea la FIRMA de la instruccion
    // esto nos tira un warning por el tema del casteo -> revisar!! (Me parece que con intptr_t se soluciona)

    int op_instruccion = (intptr_t) dictionary_get(diccionario_instrucciones, instruccion_decodificada[0]);

    switch(op_instruccion) {
        case SET:
        {
            // SET (Registro, Valor)
            printf("EJECUTANDO SET \n");

            char* registro = instruccion_decodificada[1];
            char* valor = instruccion_decodificada[2];

            usleep(config_cpu.RETARDO_INSTRUCCION * 1000);      // usleep trabaja con µs, hacemos *1000 para que sean ms
            
            asignar_a_registro(registro, valor, contexto);

            printf("EL REGISTRO %s QUEDO CON EL SIGUIENTE VALOR: %s \n", "AX", contexto->registros_cpu->AX);

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