#include "procesar_cpu_kernel.h"

// ------------------------------ PROCESAMIENTO DE CPU EN KERNEL ------------------------------ //

void procesar_cpu_kernel() {
    while(1) {
        op_code cod_op = recibir_operacion(server_cpu);
        
        // Avisamos que el proceso deja de correr, y que puede ingresar otro en la cpu
        
        switch((int)cod_op) {
            case CONTEXTO_EJECUCION:
            {
                // log_info(logger, "Me llego el codigo de operacion CONTEXTO_EJECUCION \n");

                t_contexto_ejecucion* contexto_recibido = crear_contexto();
                
                if(!recv_contexto(server_cpu, contexto_recibido)) {
                    log_error(logger, "Fallo recibiendo CONTEXTO");
                    break;
                }

                log_warning(logger,"PID: %d - PROCESO DESALOJADO \n", contexto_recibido->pid);

                // Apenas recibimos el contexto lo reasignamos al PCB que se guardo antes de mandar el proceso a RUNNING
                
                proceso_en_running->pc = contexto_recibido->pc;
                registros_copypaste(proceso_en_running->registros_cpu, contexto_recibido->registros_cpu); 

                liberar_contexto(contexto_recibido);
                break;
            }
            
            case PROCESO_DESALOJADO:
            {
                int motivo_desalojo;
                t_list* lista_parametros_recibida = list_create();
                
                // log_info(logger, "Me llego el codigo de operacion PROCESO_DESALOJADO \n");

                recv_desalojo(server_cpu, &motivo_desalojo, lista_parametros_recibida);

                // mostrar_lista(lista_parametros_recibida);

                manejar_proceso_desalojado(motivo_desalojo, lista_parametros_recibida);

                list_destroy_and_destroy_elements(lista_parametros_recibida, free);

                break;
            }
            case -1:
            {
			    log_error(logger, "El cliente CPU se desconecto. Terminando Servidor \n");
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

void manejar_proceso_desalojado(op_instruccion motivo_desalojo, t_list* lista_parametros) {
    switch((int)motivo_desalojo) {
        case YIELD:
        {
            log_info(logger, "Motivo desalojo es YIELD \n");

            proceso_en_running->tiempo_salida_running = (double)temporal_gettime(temporal);
            estimar_proxima_rafaga(proceso_en_running);

            mandar_a_ready(proceso_en_running);

            sem_post(&cpu_libre);
            break;
        }
        case EXIT:
        {
            
            log_info(logger, "Motivo desalojo es EXIT \n");         
            
            matar_proceso("SUCCESS");
            //sem_post(&cpu_libre);
            break;
        }
        case WAIT:
        {
            log_info(logger, "Motivo desalojo es WAIT \n");
            
            char* recurso_solicitado = (char*)list_get(lista_parametros, 0);

            log_info(logger, "RECURSO SOLICITADO ES %s\n", recurso_solicitado);

            t_recurso* recurso = recurso_en_lista(recurso_solicitado);


            if(recurso != NULL) {
                recurso->cantidad_disponible--;
                if(recurso->cantidad_disponible < 0)
                {
                    log_info(logger, "Proceso %d bloqueado en cola de recurso %s", proceso_en_running->pid, recurso_solicitado);
                    
                    proceso_en_running->tiempo_salida_running = (double)temporal_gettime(temporal);
                    estimar_proxima_rafaga(proceso_en_running);

                    queue_push(recurso->cola_bloqueados, proceso_en_running);      
                    sem_post(&cpu_libre);
                }
                else{
                    volver_a_running();
                }
            }
            else {
                log_error(logger, "Recurso '%s' no encontrado", recurso_solicitado);
                matar_proceso("FILE_NOT_FOUND");
            }
            break;
        }
        case SIGNAL:
        {
            log_info(logger, "Motivo desalojo es SIGNAL \n");
            
            char* recurso_solicitado = (char*)list_get(lista_parametros, 0);
            
            t_recurso* recurso = recurso_en_lista(recurso_solicitado);
            

            if(recurso != NULL) {
                log_info(logger, "RECURSO LIBERADO ES %s\n", recurso_solicitado);
                recurso->cantidad_disponible++;
                log_info(logger, "Cantidad disponible %d", recurso->cantidad_disponible);   // prueba
                 
                if(recurso->cantidad_disponible <= 0){
                    log_info(logger, "Voy a sacar a un proceso de la cola de bloqueados");
                    t_pcb* proceso = queue_pop(recurso->cola_bloqueados);
                    mandar_a_ready(proceso);
                    //sem_post(&cpu_libre);
                }
                volver_a_running();        //devuelve a running el proceso que peticiono el signal
            } 
            else {
                log_error(logger, "Recurso '%s' no encontrado", recurso_solicitado);
                matar_proceso("FILE_NOT_FOUND");
            }
            break;
        }
        case IO:
        {
            // Proceso se bloquea una cantidad de tiempo, pero no deberia bloquear al resto.
            
            args_io* argumentos_io = malloc(sizeof(args_io));
            
            argumentos_io->proceso = proceso_en_running;
            argumentos_io->tiempo = atoi((char*)list_get(lista_parametros, 0));

            
            pthread_t hilo_io;
	        pthread_create(&hilo_io, NULL, (void*)bloquear_proceso, (args_io*) argumentos_io);
	        pthread_detach(hilo_io);

            
            proceso_en_running->tiempo_salida_running = (double)temporal_gettime(temporal);
            estimar_proxima_rafaga(proceso_en_running);
            
            sem_post(&cpu_libre); // A pesar de que el proceso se bloquee la CPU estará libre, así pueden seguir ejecutando otros procesos.
            
            break;
        }
        case CREATE_SEGMENT:
        {
            int id_segmento = atoi((char*)list_get(lista_parametros, 0));
            int tamanio_segmento = atoi((char*)list_get(lista_parametros, 1));
            int pid = proceso_en_running->pid;

            // Mandarle a memoria de crear segmento junto a pid del proceso
            send_solicitud_creacion_segmento(server_memoria, pid, id_segmento, tamanio_segmento);


            op_respuesta_memoria respuesta_memoria;

            RECV_INT(server_memoria, respuesta_memoria);

            switch((int)respuesta_memoria){
                case CREACION:
                {
                    log_debug(logger, "Se creara segmento");
                    int base_segmento;
                    RECV_INT(server_memoria, base_segmento);

                    t_segmento* segmento = buscar_segmento_por_id(id_segmento, proceso_en_running->tabla_segmentos);
                    
                    segmento->direccion_base = base_segmento;
                    segmento->tamanio = tamanio_segmento; 


                    log_debug(logger, "Segmento de base %d agregado a tabla de segmentos del proceso %d", base_segmento, pid);
                    volver_a_running();

                    break;
                }
                case COMPACTACION:
                {
                    log_debug(logger, "Kernel solicitara compactacion a memoria \n");
                    
                    SEND_INT(server_memoria, SOLICITUD_COMPACTACION);
                    
                    
                    t_list* lista_recepcion_segmentos_actualizados = list_create();
                    
                    
                    // Hay que recibir todas las listas de segmentos actualizadas
                   
                    
                    if(!recv_resultado_compactacion(server_memoria, lista_recepcion_segmentos_actualizados)) {
                        log_error(logger, "Hubo un problema al recibir el resultado de la compactacion \n");
                        break;
                    }
                    
                    log_debug(logger, "TABLA ACTUALIZADA RECIBIDA!!! \n");

                    
                    log_debug(logger, "TABLA DE SEGMENTOS DE PROCESO ANTES DE COMPACTAR: \n");
                    mostrar_tabla_segmentos(proceso_en_running->tabla_segmentos);

                    
                    
                    
                    log_debug(logger, "Voy a actualizar todas mis tablas de procesos \n");

                    // Tengo que iterar la tabla recibida, y por cada pid, buscar la tabla de procesos 
                    // y ponerla en el pcb correspondiente
                    t_list_iterator* lista_it = list_iterator_create(lista_recepcion_segmentos_actualizados);
                    
                    while(list_iterator_has_next(lista_it)) {

                        t_tabla_proceso* tabla_proceso = (t_tabla_proceso*)list_iterator_next(lista_it);

                        t_pcb* proceso_a_modificar = buscar_proceso_por_pid_en_lista_global_procesos(lista_global_procesos, tabla_proceso->pid);

                        list_destroy_and_destroy_elements(proceso_a_modificar->tabla_segmentos, free);

                        proceso_a_modificar->tabla_segmentos = tabla_proceso->lista_segmentos;                        
                    }

                    list_iterator_destroy(lista_it);
    
        
                    list_destroy_and_destroy_elements(lista_recepcion_segmentos_actualizados, free);
                
    
                    send_solicitud_creacion_segmento(server_memoria, pid, id_segmento, tamanio_segmento);
                    
                    
                    // Despues de esto hay que literalmente copypastear el case CREACION
                    // Capaz que hay una forma de hacerlo mejor que copypasteando -> no se puede salir de este case y que caiga de nuevo en creacion?

                    op_respuesta_memoria respuesta;

                    RECV_INT(server_memoria, respuesta);

                    if(respuesta == CREACION) {

                        log_debug(logger, "Se creara segmento");
                        int base_segmento;
                        RECV_INT(server_memoria, base_segmento);

                        t_segmento* segmento = buscar_segmento_por_id(id_segmento, proceso_en_running->tabla_segmentos);
                        
                        segmento->direccion_base = base_segmento;
                        segmento->tamanio = tamanio_segmento; 

                        // Mostramos la tabla con el segmento creado con la compactacion
                        log_debug(logger, "Mostrando tabla actualizada recibida de memoria en la compactacion \n");
                        mostrar_tabla_segmentos(proceso_en_running->tabla_segmentos);

                        log_debug(logger, "Segmento de base %d agregado a tabla de segmentos del proceso %d", base_segmento, pid);
                        volver_a_running();

                    }

                    break;
                }
                case OUT_OF_MEMORY:
                {
                    matar_proceso("OUT_OF_MEMORY");
                    break;
                }
                default:
                {
                    log_error(logger, "Mensaje de memoria a kernel no es el adecuado");
                }
            }

            break;
        }
        case DELETE_SEGMENT:
        {
            int id_segmento = atoi((char*)list_get(lista_parametros, 0));

            send_solicitud_eliminacion_segmento(server_memoria, id_segmento, proceso_en_running->pid);
            
            t_list* tabla_segmentos_actualizada = list_create();
            
            // Recibimos la tabla actualizada
            if(!recv_tabla_segmentos(server_memoria, tabla_segmentos_actualizada)) {
                log_error(logger, "Fallo recibiendo la tabla de segmentos actualizada");
                break;
            }
            
            log_debug(logger, "Se recibio la tabla de segmentos actualizada del proceso %d", proceso_en_running->pid);

            // Debug -> mostramos la tabla cada vez que se hace un delete para chequear que se borre bien
            //mostrar_tabla_segmentos(tabla_segmentos_actualizada);
            
            list_destroy_and_destroy_elements(proceso_en_running->tabla_segmentos, free);
            proceso_en_running->tabla_segmentos = tabla_segmentos_actualizada;
            
            volver_a_running();
   
            break;
        }
        case F_OPEN:
        {
            break;
        }
        case F_CLOSE:
        {
            break;
        }
        case F_SEEK:
        {
            break;
        }
        case F_READ:
        {
            break;
        }
        case F_WRITE:
        {
            break;
        }
        case F_TRUNCATE:
        {
            break;
        }
        case MOV_IN:
        case MOV_OUT:
        {
            matar_proceso("SEG_FAULT");
            break;
        }
        default:
        {
            log_error(logger, "Motivo de desalojo no es valido\n");
            break;
        }
    }  
}


