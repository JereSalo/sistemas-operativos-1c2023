#include "mmu.h"

int traducir_direccion(int direccion_logica){
    int tam_max_segmento = config_cpu.TAM_MAX_SEGMENTO;

    int num_segmento = floor(direccion_logica / tam_max_segmento);
    int desplazamiento_segmento = direccion_logica % tam_max_segmento;

    return 0; // TODO: Retornar direccion fisica
}