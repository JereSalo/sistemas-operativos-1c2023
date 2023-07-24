#include "procesar_fs_kernel.h"

// ------------------------------ PROCESAMIENTO DE FS EN KERNEL ------------------------------ //

void procesar_fs_kernel() {
    while(1) {
        op_code cod_op = recibir_operacion(server_fs);
        
        // Avisamos que el proceso deja de correr, y que puede ingresar otro en la cpu
        switch((int)cod_op) {
            case RESPUESTA_FOPEN:
            {
                int respuesta_fs;
                char nombre_archivo[64];
                RECV_INT(server_fs, respuesta_fs);
                recv_string(server_fs, nombre_archivo);

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

                break;
            }
            case RESPUESTA_CREATE:
            {
                log_info(logger, "Se creo exitosamente el archivo \n");
                
                break;
            }
            case RESPUESTA_FTRUNCATE:
            {
                int pid;
                RECV_INT(server_fs, pid);

                // No lo busca en la lista global sino en la de bloqueados
                t_pcb* proceso = buscar_proceso_por_pid_en_lista_global_procesos(lista_bloqueados_truncate, pid);

                list_remove_element(lista_bloqueados_truncate, proceso);

                mandar_a_ready(proceso);

                break;
            }
            case RESPUESTA_FREAD:
            {
                int respuesta_fs;
                RECV_INT(server_fs, respuesta_fs);
            
                if(respuesta_fs) {   
                    int pid_recibido;
                    RECV_INT(server_fs, pid_recibido);
                    
                    log_debug(logger, "La operacion de F_READ por parte del proceso %d termino exitosamente \n", pid_recibido);

                    t_pcb* proceso = buscar_proceso_por_pid_en_lista_global_procesos(lista_bloqueados_fread_fwrite, pid_recibido);

                    list_remove_element(lista_bloqueados_fread_fwrite, proceso);

                    verificar_operaciones_terminadas(lista_bloqueados_fread_fwrite);
                    
                    mandar_a_ready(proceso);
                }
                break;
            }
            case RESPUESTA_FWRITE:
            {
                int respuesta_fs;
                RECV_INT(server_fs, respuesta_fs);
                int pid_recibido;
                RECV_INT(server_fs, pid_recibido);

                if(respuesta_fs) {
                    
                    log_debug(logger, "La operacion de F_WRITE por parte del proceso %d termino exitosamente \n", pid_recibido);

                    t_pcb* proceso = buscar_proceso_por_pid_en_lista_global_procesos(lista_bloqueados_fread_fwrite, pid_recibido);

                    list_remove_element(lista_bloqueados_fread_fwrite, proceso);

                    // Si todas las operaciones estan terminadas entonces hacemos signal del semaforo de compactacion
                    verificar_operaciones_terminadas(lista_bloqueados_fread_fwrite);

                    mandar_a_ready(proceso);
                }

                break;
            }
        }
    }
}

