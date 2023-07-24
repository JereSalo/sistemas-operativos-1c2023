#include "procesar_fs_kernel.h"

// ------------------------------ PROCESAMIENTO DE FS EN KERNEL ------------------------------ //

void procesar_fs_kernel() {
    while(1) {
        op_code cod_op = recibir_operacion(server_fs);
        
        // Avisamos que el proceso deja de correr, y que puede ingresar otro en la cpu
        
        switch((int)cod_op) {
            case CONTEXTO_EJECUCION:
            {
                break;
            }  
        }
    }
}

