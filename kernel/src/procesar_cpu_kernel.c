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
                
                list_add(proceso_en_running->recursos_asignados, recurso);
                log_debug(logger, "Recurso %s aniadido a la lista de recursos del proceso %d \n", recurso->dispositivo, proceso_en_running->pid);
                
                log_warning(logger, "PID: %d - Wait: %s - Instancias: %d \n", proceso_en_running->pid, recurso->dispositivo, recurso->cantidad_disponible); //LOG WAIT

                if(recurso->cantidad_disponible < 0)
                {
                    log_warning(logger,"PID: %d - Estado anterior: RUNNING - Estado actual: BLOCKED \n", proceso_en_running->pid); //LOG CAMBIO DE ESTADO
                    log_warning(logger,"PID: %d - Bloqueado por: %s \n", proceso_en_running->pid, recurso_solicitado);             //LOG MOTIVO DE BLOQUEO     
                    
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
                matar_proceso("RESOURCE_NOT_FOUND");
            }
            break;
        }
        case SIGNAL:
        {
            log_info(logger, "Motivo desalojo es SIGNAL \n");
            
            char* recurso_solicitado = (char*)list_get(lista_parametros, 0);
            
            t_recurso* recurso = recurso_en_lista(recurso_solicitado);
            

            if(recurso != NULL) {
                
                recurso->cantidad_disponible++;
                
                list_remove_element(proceso_en_running->recursos_asignados, recurso);
                log_debug(logger, "Recurso %s eliminado de la lista de recursos del proceso %d \n", recurso->dispositivo, proceso_en_running->pid);

                log_warning(logger, "PID: %d - Signal: %s - Instancias: %d \n", proceso_en_running->pid, recurso->dispositivo, recurso->cantidad_disponible); //LOG SIGNAL

                //log_info(logger, "Cantidad disponible %d", recurso->cantidad_disponible);   // prueba
                 
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
                matar_proceso("RESOURCE_NOT_FOUND");
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

            log_warning(logger,"PID: %d - Estado anterior: RUNNING - Estado actual: BLOCKED \n", proceso_en_running->pid);  //LOG CAMBIO DE ESTADO
            log_warning(logger,"PID: %d - Bloqueado por: I/O \n", proceso_en_running->pid);                                 //LOG MOTIVO DE BLOQUEO     
            
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


                    log_warning(logger, "PID: %d - Crear Segmento - ID: %d - Tamanio: %d \n", proceso_en_running->pid, segmento->id, segmento->tamanio); //LOG CREAR SEGMENTO
                    
                    log_debug(logger, "Segmento de base %d agregado a tabla de segmentos del proceso %d", base_segmento, pid);
                    volver_a_running();

                    break;
                }
                case COMPACTACION:
                {
                    log_warning(logger, "Compactacion: Se solicito compactacion \n"); //LOG INICIO COMPACTACION

                    SEND_INT(server_memoria, SOLICITUD_COMPACTACION);
                    
                    
                    t_list* lista_recepcion_segmentos_actualizados = list_create();
                    
                    
                    
                    
                    // Hay que recibir todas las listas de segmentos actualizadas
                   
                    
                    if(!recv_resultado_compactacion(server_memoria, lista_recepcion_segmentos_actualizados, cant_segmentos)) {
                        log_error(logger, "Hubo un problema al recibir el resultado de la compactacion \n");
                        break;
                    }
                    
                    log_debug(logger, "TABLA ACTUALIZADA RECIBIDA!!! \n");
                    
                    log_debug(logger, "TABLA DE SEGMENTOS DE PROCESO ANTES DE COMPACTAR: \n");
                    mostrar_tabla_segmentos(proceso_en_running->tabla_segmentos);

                    actualizar_tablas_segmentos(lista_recepcion_segmentos_actualizados);

                    log_debug(logger, "TABLA DE SEGMENTOS DE PROCESO DESPUES DE COMPACTAR: \n");
                    mostrar_tabla_segmentos(proceso_en_running->tabla_segmentos);

                    
                    log_warning(logger, "Se finalizo el proceso de compactacion \n"); //LOG FIN COMPACTACION


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
            
            log_warning(logger, "PID: %d - Eliminar Segmento - ID Segmento: %d \n", proceso_en_running->pid, id_segmento); //LOG ELIMINAR SEGMENTO
           
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

            log_info(logger, "Motivo desalojo es F_OPEN \n");

            char* nombre_archivo = (char*)list_get(lista_parametros, 0);


            // Buscar si existe el archivo en la TGAA
            t_tabla_global_archivos_abiertos* archivo = buscar_archivo_en_tabla_global(nombre_archivo);

            t_tabla_archivos_abiertos_proceso* archivo_proceso = malloc(sizeof(t_tabla_archivos_abiertos_proceso));

            archivo_proceso->nombre = strdup(nombre_archivo);
            archivo_proceso->puntero_archivo = 0;
            
            log_debug(logger, "Se agrega la entrada del archivo %s a la TAAP \n", nombre_archivo);
            list_add(proceso_en_running->tabla_archivos_abiertos, archivo_proceso);
            //mostrar_tabla_archivos_por_proceso(proceso_en_running->tabla_archivos_abiertos);

            log_warning(logger, "PID: %d - Abrir archivo: %s \n", proceso_en_running->pid, nombre_archivo);

            if(archivo != NULL) {

                log_info(logger, "El proceso PID %d se bloqueo porque el archivo %s se encuentra abierto por otro proceso \n", proceso_en_running->pid, nombre_archivo);
                
                queue_push(archivo->cola_bloqueados, proceso_en_running);
                
                sem_post(&cpu_libre);
            }
            else {


                log_info(logger, "El archivo %s NO se encuentra abierto por otro proceso \n", nombre_archivo);
                
                // Verifica FS si existe el archivo
                send_opcode(server_fs, SOLICITUD_ABRIR_ARCHIVO);
                send_string(server_fs, nombre_archivo);


                int respuesta_fs;
                RECV_INT(server_fs, respuesta_fs);

                if(respuesta_fs) {
                    send_opcode(server_fs, SOLICITUD_CREAR_ARCHIVO);
                    send_string(server_fs, nombre_archivo);
                }

                log_debug(logger, "Se agrega la entrada del archivo %s a la TGAA \n", nombre_archivo);
                
                // Agregamos a la TGAA

                t_tabla_global_archivos_abiertos* archivo_abierto = malloc(sizeof(t_tabla_global_archivos_abiertos));

                archivo_abierto->nombre = strdup(nombre_archivo);
                archivo_abierto->esta_abierto = 1;
                archivo_abierto->cola_bloqueados = queue_create();
                
                list_add(tabla_global_archivos_abiertos, archivo_abierto);

                volver_a_running();
            }

            break;
        }
        case F_CLOSE:
        {
            log_info(logger, "Motivo desalojo es F_CLOSE \n");

            char* nombre_archivo = (char*)list_get(lista_parametros, 0);
 
            // buscamos si el proceso realmente abrio el archivo que quiere cerrar
            //mostrar_tabla_archivos_por_proceso(proceso_en_running->tabla_archivos_abiertos);
            
            t_tabla_archivos_abiertos_proceso* archivo = buscar_archivo_en_tabla_archivos_por_proceso(proceso_en_running, nombre_archivo);

            log_debug(logger, "Encontre el archivo %s ", archivo->nombre);


            //mostrar_tabla_archivos_por_proceso(proceso_en_running->tabla_archivos_abiertos);

            if(archivo == NULL){
                log_error(logger, "El archivo no fue abierto por este proceso");
                //break;
                
                //NO SABEMOS QUE HACER ACA, SI MANDARLO A READY O MATARLO

                // probamos con mandarlo a ready
                mandar_a_ready(proceso_en_running);
                sem_post(&cpu_libre);
            }
            
            // Si esta abierto, lo sacamos de la tabla de archivos de ese proceso
            else {
                log_debug(logger, "El archivo fue abierto por este proceso anteriormente y se va a cerrar \n");
                
                list_remove_element(proceso_en_running->tabla_archivos_abiertos, archivo);
                free(archivo->nombre);
                free(archivo);

                //mostrar_tabla_archivos_por_proceso(proceso_en_running->tabla_archivos_abiertos);
                
                // Ahora hay que ver si hay otros procesos esperando para abrir ese archivo
                // Para eso lo buscamos en la TGAA

                t_tabla_global_archivos_abiertos* archivo_global = buscar_archivo_en_tabla_global(nombre_archivo);

                //log_debug(logger, "Encontre este archivo en la TGAA: %s", archivo_global->nombre);

                log_warning(logger, "PID: %d - Cerrar Archivo: %s \n", proceso_en_running->pid, nombre_archivo); //LOG CERRAR ARCHIVO


                if(queue_is_empty(archivo_global->cola_bloqueados))
                {
                    log_debug(logger, "La cola de bloqueados del archivo esta vacia");
                    
                    // Si no hay ningun proceso esperando para abrir el archivo (cola vacia) lo sacamos de la la TGGA
                    list_remove_element(tabla_global_archivos_abiertos, archivo_global);
                    queue_destroy(archivo_global->cola_bloqueados);
                    free(archivo_global->nombre);
                    free(archivo_global);
                    
                    log_debug(logger, "Entrada de archivo de la TGAA eliminada");
                }
                else {
                    log_debug(logger, "La cola de bloqueados del archivo no esta vacia");
                    
                    // Si hay procesos esperando, desbloqueamos al primero de la cola (lo ponemos en ready)
                    
                    t_pcb* proceso = queue_pop(archivo_global->cola_bloqueados);
    
                    log_debug(logger, "Primer proceso de la cola ha sido desbloqueado");

                    mandar_a_ready(proceso);
                }

                volver_a_running();
            }

            //mostrar_tabla_archivos_por_proceso(proceso_en_running->tabla_archivos_abiertos);

            break;
        }
        case F_SEEK:
        {
            char* nombre_archivo = (char*)list_get(lista_parametros, 0);
            int posicion = atoi((char*)list_get(lista_parametros, 1));

            // buscamos si el archivo esta abierto por el proceso
            t_tabla_archivos_abiertos_proceso* archivo = buscar_archivo_en_tabla_archivos_por_proceso(proceso_en_running, nombre_archivo);
            
            if(archivo == NULL){
                log_error(logger, "El archivo no fue abierto por este proceso");
                mandar_a_ready(proceso_en_running);
                sem_post(&cpu_libre);
            }
            else{
                //send_opcode(server_fs, VERIFICACION_TAMANIO);
                //SEND_INT(server_fs, posicion);
                //int respuesta_fs;
                //RECV_INT(server_fs, respuesta_fs);

                /* if(respuesta_fs) {
                    archivo->puntero_archivo = posicion;
                }
                else{
                    log_error(logger, "El puntero excede el tamanio maximo del archivo");
                } */

                // Actualizamos el puntero -> que pasa si excede al tamanio del archivo???? 
                archivo->puntero_archivo = posicion;

                log_warning(logger, "PID: %d - Actualizar puntero Archivo: %s - Puntero: %d \n", proceso_en_running->pid, nombre_archivo, archivo->puntero_archivo); //LOG ACTUALIZAR PUNTERO ARCHIVO
                
                volver_a_running();
            }
            break;
        }
        case F_READ:
        {
            char* nombre_archivo = (char*)list_get(lista_parametros, 0);
            int direccion_fisica = atoi((char*)list_get(lista_parametros, 1));
            int cantidad_bytes = atoi((char*)list_get(lista_parametros, 2));
            
            // Bloqueamos al proceso
            list_add(lista_bloqueados, proceso_en_running);
            
            sem_post(&cpu_libre);
            
            send_opcode(server_fs, SOLICITUD_LECTURA_DISCO);
            send_string(server_fs, nombre_archivo);
            SEND_INT(server_fs, cantidad_bytes);
            SEND_INT(server_fs, direccion_fisica);
            
            // Mandamos el PID para que despues el FS le devuelva al Kernel el PID del proceso que debe desbloquear
            SEND_INT(server_fs, proceso_en_running->pid);

            //Buscamos el puntero del archivo y lo mandamos a fs
            t_tabla_archivos_abiertos_proceso* archivo = buscar_archivo_en_tabla_archivos_por_proceso(proceso_en_running, nombre_archivo);
            SEND_INT(server_fs, archivo->puntero_archivo);

            // FS confirma que termino la operacion y desbloqueamos al proceso
            int respuesta_fs;
            RECV_INT(server_fs, respuesta_fs);
            
            if(respuesta_fs) {   
                int pid;
                RECV_INT(server_fs, pid);
                
                log_debug(logger, "La operacion de F_READ por parte del proceso %d termino exitosamente \n", pid);

                t_pcb* proceso = buscar_proceso_por_pid_en_lista_global_procesos(lista_bloqueados, pid);

                list_remove_element(lista_bloqueados, proceso);

                mandar_a_ready(proceso);
            }
            
            break;
        }
        case F_WRITE:
        {
            char* nombre_archivo = (char*)list_get(lista_parametros, 0);
            int direccion_fisica = atoi((char*)list_get(lista_parametros, 1));
            int cantidad_bytes = atoi((char*)list_get(lista_parametros, 2));

            // Bloqueamos al proceso
            list_add(lista_bloqueados, proceso_en_running);
            sem_post(&cpu_libre);


            send_opcode(server_fs, SOLICITUD_ESCRITURA_DISCO);
            send_string(server_fs, nombre_archivo);
            SEND_INT(server_fs, direccion_fisica);
            SEND_INT(server_fs, cantidad_bytes);
            SEND_INT(server_fs, proceso_en_running->pid);
            
            //Buscamos el puntero del archivo y lo mandamos a fs
            t_tabla_archivos_abiertos_proceso* archivo = buscar_archivo_en_tabla_archivos_por_proceso(proceso_en_running, nombre_archivo);
            SEND_INT(server_fs, archivo->puntero_archivo);
        
            // FS confirma que termino la operacion y desbloqueamos al proceso
            int respuesta_fs;
            RECV_INT(server_fs, respuesta_fs);
            int pid;
            RECV_INT(server_fs, pid);

            if(respuesta_fs) {
                
                log_debug(logger, "La operacion de F_WRITE por parte del proceso %d termino exitosamente \n", pid);

                t_pcb* proceso = buscar_proceso_por_pid_en_lista_global_procesos(lista_bloqueados, pid);

                list_remove_element(lista_bloqueados, proceso);

                mandar_a_ready(proceso);
            }
            break;
        }

        case F_TRUNCATE:
        {
            char* nombre_archivo = (char*)list_get(lista_parametros, 0);
            int tamanio = atoi((char*)list_get(lista_parametros, 1));

            send_opcode(server_fs, SOLICITUD_TRUNCAR_ARCHIVO);
            send_string(server_fs, nombre_archivo);
            SEND_INT(server_fs, tamanio);

            // Mandamos el PID para que despues el FS le devuelva al Kernel el PID del proceso que debe desbloquear
            SEND_INT(server_fs, proceso_en_running->pid);

            // Bloqueamos al proceso
            list_add(lista_bloqueados, proceso_en_running);
            sem_post(&cpu_libre);
            
            // recibimos el pid del proceso para volverlo a poner en ready
            int pid;
            RECV_INT(server_fs, pid);

            // No lo busca en la lista global sino en la de bloqueados
            t_pcb* proceso = buscar_proceso_por_pid_en_lista_global_procesos(lista_bloqueados, pid);

            list_remove_element(lista_bloqueados, proceso);

            mandar_a_ready(proceso);

            break;
        }
        case SEG_FAULT:
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


