#ifndef MEMORIA_UTILS_H
#define MEMORIA_UTILS_H

#include "shared.h"

typedef enum {
    ESCRITURA,
    LECTURA
} t_cod_orden;

typedef struct {
    t_cod_orden cod_orden;
    int direccion;
    int valor;
} t_orden;

void responder_pedido(t_orden orden);

#endif