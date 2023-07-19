#ifndef PROCESAR_KERNEL_FS_H
#define PROCESAR_KERNEL_FS_H

#include "file_system_utils.h"

void procesar_kernel_filesystem();
void agrandar_archivo(t_fcb* archivo, int tamanio, int cant_bloques_nuevos);
uint32_t buscar_proximo_bloque_libre();
void read_file_bytes(const char* filename);

#endif