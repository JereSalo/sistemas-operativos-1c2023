#include "procesar_cpu_kernel.h"

void procesar_cpu() {
    //ACA PODEMOS SACAR ESTE PARAMETRO QUE RECIBE, YA QUE EL SOCKET DE CPU ES GLOBAL -> POR AHORA NO LO SACO PORQUE NO QUIERO ROMPER NADA
    
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
            // send_solicitud_creacion_segmento(server_memoria, pid, id_segmento, tamanio_segmento);

            break;
        }
        case DELETE_SEGMENT:
        {
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