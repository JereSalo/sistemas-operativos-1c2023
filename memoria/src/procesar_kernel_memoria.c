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
                t_tabla_proceso* tabla_proceso = malloc(sizeof(tabla_proceso));
                tabla_proceso->pid = pid;
                tabla_proceso->lista_segmentos = list_create();

                
                // Agregarle a esa lista creada el segmento 0
                list_add(tabla_proceso->lista_segmentos,segmento_cero);
                log_debug(logger,"Segmento 0 cargado en tabla del proceso \n");

                // Agregar esa estructura (t_tabla_proceso) a la tabla de segmentos por proceso
                list_add(tabla_segmentos_por_proceso, tabla_proceso);

                // Al kernel se le devuelve en este caso la tabla interna del proceso
                send_tabla_segmentos(cliente_kernel, tabla_proceso->lista_segmentos);
                
                break;
            }
            case SOLICITUD_CREACION_SEGMENTO:
            {
                // Recibir pid, id_segmento y tamanio_segmento
                int pid;
                int id_segmento;
                int tamanio_segmento;

                recv_solicitud_creacion_segmento(cliente_kernel, &pid, &id_segmento, &tamanio_segmento);

                // Mandarle tamanio al algoritmo de busqueda y ver si encuentra hueco o devuelve NULL.
                t_hueco* hueco = obtener_hueco_libre(tamanio_segmento);
                if(hueco == NULL){
                    if(espacio_restante_memoria() >= tamanio_segmento){
                        SEND_INT(cliente_kernel, COMPACTACION);
                    }
                    else{
                        SEND_INT(cliente_kernel, OUT_OF_MEMORY);
                    }
                }
                else{
                    // Camino feliz :D
                    // Crear segmento y agregarlo a lista global de segmentos
                    // Modificar tabla de huecos
                    // Esta funcion hace ambas cosas y retorna el segmento creado
                    t_segmento* segmento_creado = crear_segmento(id_segmento, hueco->direccion_base_hueco, tamanio_segmento);

                    // Agregar tambien segmento con el PID a la tabla de segmentos por proceso.
                    agregar_segmento(segmento_creado, pid);
                    
                    // Mandarle a Kernel la base del nuevo segmento
                    SEND_INT(cliente_kernel, segmento_creado->direccion_base);
                }
            }
            case SOLICITUD_COMPACTACION:
            {
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