#include "procesar_cpu_kernel.h"

void procesar_cpu(void* void_server_cpu) {
    //ACA PODEMOS SACAR ESTE PARAMETRO QUE RECIBE, YA QUE EL SOCKET DE CPU ES GLOBAL -> POR AHORA NO LO SACO PORQUE NO QUIERO ROMPER NADA
    server_cpu = (intptr_t) void_server_cpu;
    
    while(1) {
        op_code cod_op = recibir_operacion(server_cpu);
        
        // Avisamos que el proceso deja de correr, y que puede ingresar otro en la cpu
        
        switch((int)cod_op) {
            case CONTEXTO_EJECUCION:
            {
                // log_info(logger, "Me llego el codigo de operacion CONTEXTO_EJECUCION \n");

                t_contexto_ejecucion* contexto_recibido = malloc(sizeof(t_contexto_ejecucion));
                recv_contexto(server_cpu, contexto_recibido);

                log_warning(logger,"PID: %d - PROCESO DESALOJADO \n", contexto_recibido->pid);

                
                // Apenas recibimos el contexto lo reasignamos al PCB que se guardo antes de mandar el proceso a RUNNING
                
                proceso_en_running->pc = contexto_recibido->pc;
                proceso_en_running->registros_cpu = contexto_recibido->registros_cpu;

                // free(contexto_recibido->registros_cpu); // ESTE NO VA PORQUE ESTOY LIBERANDO EL ESPACIO EN MEMORIA EN DONDE ESTAN LOS REGISTROS.
                // list_destroy_and_destroy_elements(contexto_recibido->instrucciones, free);
                // free(contexto_recibido);
                break;
                
            }
            
            case PROCESO_DESALOJADO:
            {
                int motivo_desalojo;
                t_list* lista_parametros_recibida = list_create();
                
                // log_info(logger, "Me llego el codigo de operacion PROCESO_DESALOJADO \n");

                recv_desalojo(server_cpu, &motivo_desalojo, lista_parametros_recibida);

                // mostrar_lista(lista_parametros_recibida);


                //log_info(logger, "MOTIVO DE DESALOJO: %d \n", motivo_desalojo);


                // Aca deberiamos hacer un switch nuevo para preguntar que se debe hacer segun el motivo que se recibio
                manejar_proceso_desalojado(motivo_desalojo, lista_parametros_recibida);

                
                // El semaforo debe ir al final del case debido a que la variable proceso_en_running debe ser la misma hasta 
                // que vuelva a encolar en ready, en manejar_proceso_desalojado, una vez que hace esto ya puede liberarse la cpu con el semaforo
                // Si el semaforo se pone antes, el planificador (que esta en otro hilo) va a pisar el proceso en running (variable global)
                // Al toque roque al pique enrique
                // sem_post(&cpu_libre); // En algunos casos (Wait) puede que la cpu no se libere para otro proceso. Asi que esto aca creo que no va.

                // sem_post(&maximo_grado_de_multiprogramacion); // ESTO NO VA

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

            volver_a_encolar_en_ready(proceso_en_running);

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
                    log_info(logger, "Voy a volver a running XD \n");
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
                    volver_a_encolar_en_ready(proceso);
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
            args_io* argumentos_io = malloc(sizeof(argumentos_io));
            
            argumentos_io->proceso = proceso_en_running;
            argumentos_io->tiempo = atoi((char*)list_get(lista_parametros, 0));

            pthread_t hilo_io;
	        pthread_create(&hilo_io, NULL, (void*)bloquear_proceso, (args_io*) argumentos_io);
	        pthread_detach(hilo_io);

            //proceso_en_running->tiempo_salida_running = time(NULL);
            //estimar_proxima_rafaga(proceso_en_running);
            sem_post(&cpu_libre); // A pesar de que el proceso se bloquee la CPU estará libre, así pueden seguir ejecutando otros procesos.
            break;
        }
    }  
}