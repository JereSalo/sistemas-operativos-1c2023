#include "ejecutar.h"

// ------------------------------ EJECUCION DE PROCESOS ------------------------------ //

t_list* lista_parametros;
int desalojado = 0;
int seg_fault = 0;

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
    int motivo_desalojo;

    if(seg_fault){
        log_info(logger, "PID: %d - Instruccion %s ha producido un SEG_FAULT, desalojando proceso... \n", contexto->pid, instruccion);
        motivo_desalojo = SEG_FAULT;
        seg_fault = 0;
    }
    else{
        log_info(logger, "PID: %d - Instruccion %s a ejecutar por parte del Kernel \n", contexto->pid, instruccion);
        motivo_desalojo = (intptr_t)dictionary_get(diccionario_instrucciones, instruccion_decodificada[0]); 
    }

    send_contexto(cliente_kernel, contexto);
    send_desalojo(cliente_kernel, motivo_desalojo, lista_parametros);

    list_destroy_and_destroy_elements(lista_parametros, free);
    string_array_destroy(instruccion_decodificada);
}


void ejecutar_instruccion(char** instruccion_decodificada, t_contexto_ejecucion* contexto) {
    char* nemonico_instruccion = instruccion_decodificada[0];  //nemonico: palabra que sustituye a un codigo de operacion. pertenece a la memoria.

    log_warning(logger, "PID: %d - Ejecutando %s \n", contexto->pid, nemonico_instruccion); //LOG INSTRUCCION EJECUTADA

    int op_instruccion = (intptr_t) dictionary_get(diccionario_instrucciones, nemonico_instruccion);

    switch(op_instruccion) {
        case SET: // SET (Registro, Valor)
        {
            char* registro = instruccion_decodificada[1];
            char* valor = instruccion_decodificada[2];
            
            usleep(config_cpu.RETARDO_INSTRUCCION * 1000);      // usleep trabaja con µs, hacemos *1000 para que sean ms
            
            asignar_a_registro(registro, valor, contexto->registros_cpu);

            //printf("EL REGISTRO %s QUEDO CON EL SIGUIENTE VALOR: %.*s \n", "AX", 4, contexto->registros_cpu->AX);
            

            break;
        }
        case MOV_IN: // MOV_IN (Registro, Dirección Lógica) -> LECTURA DE MEMORIA
        {
            char* registro = instruccion_decodificada[1];
            int direccion_logica = atoi(instruccion_decodificada[2]);
            int num_segmento;

            int longitud_registro = obtener_longitud_registro(registro);
            int direccion_fisica = obtener_direccion(direccion_logica, contexto, longitud_registro, &num_segmento);



            if(direccion_fisica == -1){
                desalojado = 1;
                seg_fault = 1;
                break;
            }
            
           
	        // Leer valor de memoria correspondiente a direccion_fisica

            send_peticion_lectura(server_memoria, direccion_fisica, longitud_registro);
            SEND_INT(server_memoria, contexto->pid);

            char valor[64];

            recv_string(server_memoria, valor);
            
            log_warning(logger, "PID: %d - Accion: LEER - Segmento: %d - Direccion fisica: %d - Valor leido: %s",contexto->pid, num_segmento, direccion_fisica, valor); //LOG ACCESO A MEMORIA
            
            asignar_a_registro(registro, valor, contexto->registros_cpu);

            break;
        }
        case MOV_OUT: // MOV_OUT (Dirección Lógica, Registro) -> ESCRITURA DE REGISTRO A MEMORIA
        {
            int direccion_logica = atoi(instruccion_decodificada[1]);
            char* registro = instruccion_decodificada[2];
            int num_segmento;

            int longitud_registro = obtener_longitud_registro(registro);
            int direccion_fisica = obtener_direccion(direccion_logica, contexto, longitud_registro, &num_segmento);


            if(direccion_fisica == -1){
                desalojado = 1;
                seg_fault = 1;
                break;
            }

            char* valor_leido = leer_de_registro(registro,contexto->registros_cpu);

            send_peticion_escritura(server_memoria, direccion_fisica, longitud_registro, valor_leido);
            SEND_INT(server_memoria, contexto->pid);

            log_warning(logger, "PID: %d - Accion: ESCRIBIR - Segmento: %d - Direccion fisica: %d - Valor escrito: %s",contexto->pid, num_segmento, direccion_fisica, valor_leido); //LOG ACCESO A MEMORIA

            char confirmacion[5];
            recv_string(server_memoria, confirmacion);
            
            if(string_equals_ignore_case(confirmacion, "OK")) 
            {
                log_info(logger, "Escritura en memoria exitosa \n");
            }
            
            free(valor_leido);
            
            break;
        }
        case YIELD: // YIELD
        case EXIT: // EXIT
        {   
            desalojado = 1;
            break;        
        }
        case IO: // IO (Tiempo)
        case WAIT: // WAIT (Recurso)
        case SIGNAL: // SIGNAL (Recurso)
        case DELETE_SEGMENT: // DELETE_SEGMENT (Id Segmento)
        case F_OPEN: // F_OPEN (Nombre Archivo)
        case F_CLOSE: // F_CLOSE (Nombre Archivo)
        {
            list_add(lista_parametros, strdup(instruccion_decodificada[1]));   

            desalojado = 1;
            
            break;   
        }
        case CREATE_SEGMENT: // CREATE_SEGMENT (Id del Segmento, Tamaño)
        case F_SEEK: // F_SEEK (Nombre Archivo, Posición)
        case F_TRUNCATE: // F_TRUNCATE (Nombre Archivo, Tamaño)
        {
            list_add(lista_parametros, strdup(instruccion_decodificada[1]));
            list_add(lista_parametros, strdup(instruccion_decodificada[2]));

            desalojado = 1;

            break;
        }
        case F_READ: // F_READ (Nombre Archivo, Dirección Lógica, Cantidad de Bytes)
        case F_WRITE: // F_WRITE (Nombre Archivo, Dirección Lógica, Cantidad de bytes)
        {
            char* nombre_archivo = instruccion_decodificada[1];
            int direccion_logica = atoi(instruccion_decodificada[2]);
            int cantidad_bytes = atoi(instruccion_decodificada[3]);
            int num_segmento;

            int direccion_fisica = obtener_direccion(direccion_logica, contexto, cantidad_bytes, &num_segmento);

            if(direccion_fisica == -1){
                desalojado = 1;
                seg_fault = 1;
                break;
            }

            char* direccion_fisica_string = string_itoa(direccion_fisica);

            list_add(lista_parametros, strdup(nombre_archivo));
            list_add(lista_parametros, direccion_fisica_string);
            list_add(lista_parametros, strdup(instruccion_decodificada[3]));

            desalojado = 1;

            break;
        }
        default: {
            log_error(logger, "INSTRUCCION DESCONOCIDA");
        }
    }
}
