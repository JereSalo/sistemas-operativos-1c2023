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

                t_segmento* segmento;
                t_tabla_proceso* proceso;
                t_hueco* hueco;
                
                // Estas dos nos sirven para saber donde eliminar en la tabla global de segmentos y para crear el hueco con el mismo tamanio que el segmento eliminado
                int direccion_base;
                int tamanio;

                int id_segmento;
                int pid;
            
                // Recibimos la solicitud de eliminacion del segmento
                if(!recv_solicitud_eliminacion_segmento(cliente_kernel, &id_segmento, &pid)) {
                    log_error(logger, "Fallo en la solicitud de eliminacion de segmento \n");
                    break;
                }
                
                // Buscamos el segmento dentro de la lista global de segmentos
                // Le mandamos proceso asi despues tenemos conocimiento del proceso del que se trata -> lo necesitamos para despues eliminar dicho segmento 
                segmento = buscar_segmento_en_tabla_por_proceso(tabla_segmentos_por_proceso, &proceso, pid, id_segmento);
        
                // Sacamos la direccion base de ese segmento para despues eliminar ese segmento en la lista global de segementos
                // Tambien la direccion base nos sirve para crear el hueco que va a dejar libre ese segmento
                direccion_base = segmento->direccion_base;
                
                // Obtenemos el tamanio de ese segmento para despues crear el hueco libre con ese mismo tamanio
                tamanio = segmento->tamanio;
                
               
                // Eliminamos el segmento de la tabla de segmentos por proceso
                eliminar_segmento_de_tabla(proceso->lista_segmentos, segmento, "PROCESO", pid);
        
            
                // Buscamos el segmento dentro de la lista global de segmentos
                segmento = buscar_segmento_en_tabla_global(lista_global_segmentos, direccion_base);


                // Eliminamos ese segmento de la lista global de segmentos
                eliminar_segmento_de_tabla(lista_global_segmentos, segmento, "GLOBAL", pid);

                
                // Enviamos a Kernel la tabla de segmentos actualizada
                send_tabla_segmentos(cliente_kernel, proceso->lista_segmentos);
                log_debug(logger, "Envio tabla de procesos actualizada a Kernel \n");

                // Creamos y consolidamos en caso de que haya huecos aledanios
                hueco = crear_y_consolidar_huecos(direccion_base, tamanio);

                // Agregamos el hueco a la tabla de huecos libres
                agregar_hueco(hueco);

                mostrar_tabla_huecos(tabla_huecos);

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



