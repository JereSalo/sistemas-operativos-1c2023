#include "procesar_kernel_fs.h"

void procesar_kernel_filesystem(){
    while(1) {
        op_code cod_op = recibir_operacion(cliente_kernel);

        switch((int)cod_op) {
            case SOLICITUD_LECTURA_DISCO:
                break;
            case SOLICITUD_ESCRITURA_DISCO:
                break;
            case SOLICITUD_CREAR_ARCHIVO:
                break;
            case SOLICITUD_ABRIR_ARCHIVO:
                break;
            case SOLICITUD_TRUNCAR_ARCHIVO:
                break;
        }
    }
}