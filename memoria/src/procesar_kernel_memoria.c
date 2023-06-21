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
                    log_debug(logger, "Se realizara la creacion");
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
                    // Cuidado aca con posible condicion de carrera por hacer 2 send distintos en vez de uno solo. (no creo que sea posible igual, es teórico nomas)
                }

                break;
            }
            case SOLICITUD_ELIMINACION_SEGMENTO:
            {
                log_debug(logger, "Solicitud de eliminacion de segmento recibida");

                int id_segmento;
                int pid;

                // Estas dos nos sirven para saber donde eliminar en la tabla global de segmentos y para crear el hueco con el mismo tamanio que el segmento eliminado
                int direccion_base;
                int tamanio;

                recv_solicitud_eliminacion_segmento(cliente_kernel, &id_segmento, &pid);

                // Primero eliminamos de la tabla por proceso
                t_tabla_proceso* proceso = buscar_proceso_por_pid(tabla_segmentos_por_proceso, pid);

                t_segmento* segmento = buscar_segmento_por_id(id_segmento, proceso->lista_segmentos);

                direccion_base = segmento->direccion_base;

                list_remove_element(proceso->lista_segmentos, segmento);
        

                // Despues eliminamos de la tabla global de segmentos
                segmento = buscar_segmento_por_base(direccion_base, lista_global_segmentos);

                tamanio = segmento->tamanio;

                list_remove_element(lista_global_segmentos, segmento);


                t_hueco* hueco = crear_hueco(direccion_base, tamanio);


                // Buscamos un potencial hueco anterior
                t_hueco* hueco_aledanio_1 = buscar_hueco_por_final(direccion_base - 1);

                // Para hacer esto tendriamos que guardar por cada hueco su direccion de finalizacion
                // Esto nos va a servir para buscar un hueco anterior

                // Buscamos un potencial hueco siguiente
                t_hueco* hueco_aledanio_2 = buscar_hueco_por_base(direccion_base + tamanio);


                hueco = consolidar_huecos(hueco, hueco_aledanio_1, hueco_aledanio_2);            
                
                // Agregamos el hueco consolidado a la tabla de huecos libres
                agregar_hueco(hueco);

                


                break;
            }
            case SOLICITUD_COMPACTACION:
            {
                log_debug(logger, "Solicitud de compactacion recibida");
                //TODO
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

