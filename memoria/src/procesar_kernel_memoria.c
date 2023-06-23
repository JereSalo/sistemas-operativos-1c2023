#include "procesar_kernel_memoria.h"

void procesar_kernel_memoria() {
    while(1) {
        op_code cod_op = recibir_operacion(cliente_kernel);
        
        // Avisamos que el proceso deja de correr, y que puede ingresar otro en la cpu
        
        switch((int)cod_op) {
            case SOLICITUD_TABLA_NEW:
            {
                log_debug(logger,"Recibi solicitud de Tabla de un proceso Nuevo \n");

                int pid;
                RECV_INT(cliente_kernel, pid);

                // Crear estructura t_tabla_proceso (con pid y lista de tabla de segmentos del proceso)
                // Cargarle a la estructura el pid del proceso y crear la lista que va a tener adentro
                t_tabla_proceso* tabla_proceso = malloc(sizeof(t_tabla_proceso));
                tabla_proceso->pid = pid;
                tabla_proceso->lista_segmentos = list_create();

                // Agregar esa estructura (t_tabla_proceso) a la tabla de segmentos por proceso
                list_add(tabla_segmentos_por_proceso, tabla_proceso);

                agregar_segmento(segmento_cero, pid);

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
                if(hueco == NULL){
                    if(espacio_restante_memoria() >= tamanio_segmento){
                        log_debug(logger, "Se necesita compactacion");
                        SEND_INT(cliente_kernel, COMPACTACION);
                    }
                    else{
                        log_debug(logger, "No hay memoria");
                        SEND_INT(cliente_kernel, OUT_OF_MEMORY);
                    }
                }
                else{
                    log_debug(logger, "Se realizara la creacion \n");
                    // Camino feliz :D
                    // Crear segmento y agregarlo a lista global de segmentos
                    // Modificar tabla de huecos
                    // Esta funcion hace ambas cosas y retorna el segmento creado
                    t_segmento* segmento_creado = crear_segmento(id_segmento, hueco->direccion_base, tamanio_segmento);

                    // Agregar tambien segmento con el PID a la tabla de segmentos por proceso.
                    agregar_segmento(segmento_creado, pid);
                    
                    // Mandarle a Kernel la base del nuevo segmento
                    SEND_INT(cliente_kernel, CREACION);
                    SEND_INT(cliente_kernel, segmento_creado->direccion_base);
                    //send_base_segmento();
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
                
                delete_segment(pid, id_segmento);

                // Debug
                mostrar_tabla_huecos(tabla_huecos);

                // Enviamos a Kernel la tabla de segmentos actualizada
                t_tabla_proceso* tabla_proceso = buscar_proceso_por_pid(tabla_segmentos_por_proceso, pid);
                send_tabla_segmentos(cliente_kernel, tabla_proceso->lista_segmentos);
                
                log_debug(logger, "Envio tabla de procesos actualizada a Kernel \n");

                break;
            }
            case SOLICITUD_COMPACTACION:
            {
                log_debug(logger, "Solicitud de compactacion recibida \n");
                
                // Recorrer la lista global de segmentos y mover todos al final de donde termina cada uno salvo el primero
                t_segmento* ultimo_segmento = mover_segmentos();


                // Liberamos (eliminamos) la lista de huecos libres
                list_clean_and_destroy_elements(tabla_huecos, free);                
                
                
                // Creamos el hueco libre resultante de la compactacion y lo agregamos a la lista
                int direccion_base_hueco = ultimo_segmento->direccion_base + ultimo_segmento->tamanio;
                int tamanio_hueco = config_memoria.TAM_MEMORIA - direccion_base_hueco;
                
                crear_y_agregar_hueco(direccion_base_hueco, tamanio_hueco);

                // Debug
                mostrar_tabla_huecos(tabla_huecos);

                break;
            }
            case SOLICITUD_LIBERAR_MEMORIA:
            {
                log_debug(logger,"\nRecibi solicitud para Liberar Memoria de un proceso \n");
                int pid;
                RECV_INT(cliente_kernel, pid);

                t_tabla_proceso* tabla_proceso = buscar_proceso_por_pid(tabla_segmentos_por_proceso, pid);

                // Yo haría un delete_segment para cada uno de los segmentos del proceso (menos segmento 0)
                void borrar_segmento(void* segmento){
                    int id_segmento = ((t_segmento*)segmento)->id;
                    if(id_segmento != 0)
                        delete_segment(pid, id_segmento);
                }

                list_iterate(tabla_proceso->lista_segmentos, borrar_segmento);

                // tabla_proceso va a quedar con pid y la lista con el segmento 0.
                // Ahí simplemente libero tabla_proceso
                list_remove_element(tabla_segmentos_por_proceso, tabla_proceso);
                list_destroy(tabla_proceso->lista_segmentos);
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
    }
}


void delete_segment(int pid, int id_segmento){
    t_tabla_proceso* tabla_proceso = buscar_proceso_por_pid(tabla_segmentos_por_proceso, pid);
    t_segmento* segmento = buscar_segmento_por_id(id_segmento, tabla_proceso->lista_segmentos);

    // Creamos hueco con base y tamaño del segmento a eliminar. Si hay hueco aledaño consolidamos.
    crear_y_consolidar_huecos(segmento->direccion_base, segmento->tamanio);

    // Sacamos segmento de ambas listas y liberamos memoria.
    list_remove_element(lista_global_segmentos, segmento);
    list_remove_element(tabla_proceso->lista_segmentos, segmento);
    free(segmento);
}


