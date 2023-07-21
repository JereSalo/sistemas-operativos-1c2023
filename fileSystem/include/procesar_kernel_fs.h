#ifndef PROCESAR_KERNEL_FS_H
#define PROCESAR_KERNEL_FS_H

#include "file_system_utils.h"

void procesar_kernel_filesystem();
void agrandar_archivo(t_fcb* archivo, int tamanio);
uint32_t buscar_proximo_bloque_libre();
void read_file_bytes(const char* filename);
void achicar_archivo(t_fcb* archivo, int tamanio_nuevo);
int calcular_posicion_ultimo_puntero(t_fcb* archivo);
int find_last_data_position(void* buffer, size_t buffer_size);

#endif