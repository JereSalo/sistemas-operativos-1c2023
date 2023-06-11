#include "ejecutar.h"

t_list* lista_parametros;
int desalojado = 0;

void ejecutar_proceso(t_contexto_ejecucion* contexto) {
    char* instruccion;
    char** instruccion_decodificada;
    lista_parametros = list_create();

    while(!desalojado) {
        // Fetch: buscamos la proxima instruccion dada por el PC
        instruccion = list_get(contexto->instrucciones, contexto->pc);
        
        // Decode: interpretamos la instruccion (que intruccion es y que parametros lleva)
        instruccion_decodificada = string_split(instruccion, " "); // recordar que string_split hace que ult elemento sea NULL
        
        //lista_parametros = list_create();
        
        ejecutar_instruccion(instruccion_decodificada, contexto);

        if (!desalojado) {
            log_info(logger, "PID: %d - Instruccion %s finalizada \n", contexto->pid, instruccion);
            string_array_destroy(instruccion_decodificada);
        }

        contexto->pc++;
    }

    // Si es desalojado =>
    desalojado = 0;

    log_info(logger, "PID: %d - Instruccion %s a ejecutar por parte del Kernel \n", contexto->pid, instruccion);

    send_contexto(cliente_kernel, contexto);
    send_desalojo(cliente_kernel, (intptr_t)dictionary_get(diccionario_instrucciones, instruccion_decodificada[0]), lista_parametros);

    list_destroy_and_destroy_elements(lista_parametros, free);
    string_array_destroy(instruccion_decodificada);
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
            list_add(lista_parametros, strdup(instruccion_decodificada[1]));   

            desalojado = 1;
            
            break;   
        }
        default: {
            log_error(logger, "INSTRUCCION DESCONOCIDA");
        }
    }
}