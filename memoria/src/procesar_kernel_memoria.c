#include "procesar_kernel_memoria.h"

void procesar_kernel_memoria() {
    SEND_INT(cliente_kernel, config_memoria.CANT_SEGMENTOS);

    while(1) {
        op_code cod_op = recibir_operacion(cliente_kernel);
        
        // Avisamos que el proceso deja de correr, y que puede ingresar otro en la cpu
        pthread_mutex_lock(&mutex_falopa);
        
        switch((int)cod_op) {
            case SOLICITUD_TABLA_NEW:
            {
                log_debug(logger,"Recibi solicitud de Tabla de un proceso Nuevo \n");

                int pid;
                RECV_INT(cliente_kernel, pid);
                
                log_warning(logger, "Creacion de proceso PID: %d \n", pid); //LOG CREACION DE PROCESO

                // Crear estructura t_tabla_proceso (con pid y lista de tabla de segmentos del proceso)
                // Cargarle a la estructura el pid del proceso y crear la lista que va a tener adentro
                t_tabla_proceso* tabla_proceso = crear_tabla_proceso(pid);

                // Agregar esa estructura (t_tabla_proceso) a la tabla de segmentos por proceso
                list_add(tabla_segmentos_por_proceso, tabla_proceso);

                // Al kernel se le devuelve en este caso la tabla interna del proceso
                send_tabla_segmentos(cliente_kernel, tabla_proceso->lista_segmentos);
                
                break;
            }
            case SOLICITUD_CREACION_SEGMENTO:
            {
                log_debug(logger, "Solicitud de creacion de segmento recibida");
                // Recibir pid, id_segmento y tamanio_segmento
                int pid;
                int id_segmento;
                int tamanio_segmento;
                
                recv_solicitud_creacion_segmento(cliente_kernel, &pid, &id_segmento, &tamanio_segmento);

                // log_debug(logger, "PID %d", pid);
                // log_debug(logger, "ID SEGMENTO %d", id_segmento);
                // log_debug(logger, "TAMANIO SEGMENTO %d", tamanio_segmento);
                // Mandarle tamanio al algoritmo de busqueda y ver si encuentra hueco o devuelve NULL.
                t_hueco* hueco = obtener_hueco_libre(tamanio_segmento);
                
                
                if(hueco != NULL)
                    log_debug(logger, "Obtuve hueco - Base: %d - Tamanio: %d \n", hueco->direccion_base, hueco->tamanio);
                
                if(hueco == NULL){
                    if(espacio_restante_memoria() >= tamanio_segmento){
                        log_debug(logger, "Se necesita compactacion");
                        SEND_INT(cliente_kernel, COMPACTACION);
                    }
                    else{
                        log_debug(logger, "No hay suficiente espacio en memoria");
                        SEND_INT(cliente_kernel, OUT_OF_MEMORY);
                    }
                }
                else{
                    log_debug(logger, "Se realizara la creacion \n");
                    // Es una vil mentira la creacion, solo modifica un segmento existente

                    
                    // Tengo que obtener un segmento en particular dado PID y ID_SEGMENTO y modificar su BASE y TAMAÑO
                    t_tabla_proceso* tabla_proceso = buscar_proceso_por_pid(tabla_segmentos_por_proceso, pid);
                    t_segmento* segmento = buscar_segmento_por_id(id_segmento, tabla_proceso->lista_segmentos);

                    // Modificar base y tamaño de segmento, y modificar base y tamaño de hueco
                    segmento->direccion_base = hueco->direccion_base;
                    segmento->tamanio = tamanio_segmento;
                    
                    log_warning(logger, "PID: %d - Crear Segmento: %d - Base: %d - Tamanio: %d \n", pid, segmento->id, segmento->direccion_base, segmento->tamanio); //LOG CREACION DE SEGMENTO
                                       
                    hueco->direccion_base += segmento->tamanio;
                    hueco->tamanio -= segmento->tamanio;

                    // log_debug(logger, "Direccion base segmento creado: %d", segmento->direccion_base);

                    if(hueco->tamanio == 0){
                        log_debug(logger, "Hueco eliminado \n");
                        list_remove_element(tabla_huecos, hueco);
                        free(hueco);
                    }
                  
                    
                    
                    // Mandarle a Kernel la base del nuevo segmento
                    log_debug(logger, "Se le manda al Kernel la base del segmento creado \n");
                    // SEND_INT(cliente_kernel, CREACION);
                    // SEND_INT(cliente_kernel, segmento->direccion_base);
                    
                    send_base_segmento_creado(cliente_kernel, segmento->direccion_base);
                    // Cuidado aca con posible condicion de carrera por hacer 2 send distintos en vez de uno solo. (no creo que sea posible igual, es teórico nomas)
                }

                break;
            }
            case SOLICITUD_ELIMINACION_SEGMENTO:
            {
                log_debug(logger, "Solicitud de eliminacion de segmento recibida \n");

                int id_segmento;
                int pid;
            
                // Recibimos la solicitud de eliminacion del segmento
                if(!recv_solicitud_eliminacion_segmento(cliente_kernel, &id_segmento, &pid)) {
                    log_error(logger, "Fallo en la solicitud de eliminacion de segmento \n");
                    break;
                }
                
                t_tabla_proceso* tabla_proceso = buscar_proceso_por_pid(tabla_segmentos_por_proceso, pid);
                t_segmento* segmento = buscar_segmento_por_id(id_segmento, tabla_proceso->lista_segmentos);

                // log_debug(logger, "ID del segmento a eliminar: %d", segmento->id);
                // log_debug(logger, "Tamanio del segmento a eliminar: %d", segmento->tamanio);
                // log_debug(logger, "Direccion base segmento a eliminar: %d", segmento->direccion_base);

                // Creamos hueco con base y tamaño del segmento a eliminar. Si hay hueco aledaño consolidamos.
                crear_y_consolidar_huecos(segmento->direccion_base, segmento->tamanio);

                log_warning(logger, "PID: %d - Eliminar Segmento: %d - Base: %d - Tamanio: %d \n", pid, segmento->id, segmento->direccion_base, segmento->tamanio); //LOG ELIMINACION DE SEGMENTO
                
                // Acá se produce la "eliminación" del segmento
                segmento->direccion_base = -1;
                segmento->tamanio = 0;

                // Debug
                //mostrar_tabla_huecos(tabla_huecos);

                send_tabla_segmentos(cliente_kernel, tabla_proceso->lista_segmentos);
                
                log_debug(logger, "Envio tabla de procesos actualizada a Kernel \n");

                break;
            }
            case SOLICITUD_COMPACTACION:
            {
                log_warning(logger, "Solicitud de compactacion \n"); //LOG INICIO COMPACTACION
                
                // Recorrer la lista global de segmentos y mover todos al final de donde termina cada uno salvo el primero
                t_segmento* ultimo_segmento = mover_segmentos();

                // Liberamos (eliminamos) la lista de huecos libres
                list_clean_and_destroy_elements(tabla_huecos, free);                
                
                // Creamos el hueco libre resultante de la compactacion y lo agregamos a la lista
                int direccion_base_hueco = ultimo_segmento->direccion_base + ultimo_segmento->tamanio;
                int tamanio_hueco = config_memoria.TAM_MEMORIA - direccion_base_hueco;
                
                crear_y_agregar_hueco(direccion_base_hueco, tamanio_hueco);

                // Mandamos a Kernel las tablas actualizadas
                
                
                log_debug(logger, "Se va a mandar el resultado de la compactacion a Kernel \n");
                usleep(config_memoria.RETARDO_COMPACTACION * 1000); // Retardo respuesta compactacion
                send_resultado_compactacion(cliente_kernel, tabla_segmentos_por_proceso, config_memoria.CANT_SEGMENTOS);


                mostrar_resultado_compactacion(tabla_segmentos_por_proceso);
                
                // Debug
                //mostrar_tabla_huecos(tabla_huecos);

                // Debug
                //leer_memoria();


                break;
            }
            case SOLICITUD_LIBERAR_MEMORIA:
            {
                int pid;
                RECV_INT(cliente_kernel, pid);

                log_warning(logger, "Eliminacion de proceso PID: %d \n", pid); //LOG ELIMINACION DE PROCESO


                t_tabla_proceso* tabla_proceso = buscar_proceso_por_pid(tabla_segmentos_por_proceso, pid);

                // Sacamos el segmento 0 porque lo queremos mucho y no lo vamos a destruir
                list_remove(tabla_proceso->lista_segmentos, 0);

                // Tengo que remover de la lista global de segmentos todos los segmentos de este proceso, qué hago?
                // Si lo quiero hacer bien tengo que iterar tabla_proceso->lista_segmentos y sacar cada elemento de la lista global

                void sacar_de_lista_global(void* segmento){
                    list_remove_element(lista_global_segmentos, segmento);
                }

                list_iterate(tabla_proceso->lista_segmentos, sacar_de_lista_global);

                list_remove_element(tabla_segmentos_por_proceso, tabla_proceso);

                // Hacemos cacona al resto
                list_destroy_and_destroy_elements(tabla_proceso->lista_segmentos, free);

                free(tabla_proceso);

                break;
            }
            case -1:
            {
			    log_error(logger, "El cliente KERNEL se desconecto. Terminando Servidor \n");
			    return;
            }
		    default:
            {
			    log_error(logger,"Operación desconocida. Hubo un problemita! \n");
			    break;
            }
        }
        pthread_mutex_unlock(&mutex_falopa);
    }
}




// Crea la tabla y los segmentos que están contenidos en esta, agregándolos también a la lista global de segmentos
t_tabla_proceso* crear_tabla_proceso(int pid){
    t_tabla_proceso* tabla_proceso = malloc(sizeof(t_tabla_proceso));
    tabla_proceso->pid = pid;
    tabla_proceso->lista_segmentos = list_create();
    list_add(tabla_proceso->lista_segmentos, segmento_cero);
    
    for(int id_seg = 1; id_seg < config_memoria.CANT_SEGMENTOS; id_seg++){
        t_segmento* segmento = crear_segmento(id_seg, -1, 0);
        list_add(tabla_proceso->lista_segmentos, segmento);
    }

    // printf("Tabla segmentos en crear tabla proceso\n");
    // mostrar_tabla_segmentos(tabla_proceso->lista_segmentos);

    return tabla_proceso;
}