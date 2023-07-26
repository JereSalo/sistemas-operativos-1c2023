#include "memoria_utils.h"

// ------------------------------ VARIABLES GLOBALES ------------------------------ //


pthread_mutex_t mutex_falopa;

// LOGGER Y CONFIG
t_log* logger;
t_config *config;
t_memoria_config config_memoria;

// SOCKETS
int cliente_kernel;
int cliente_cpu;
int cliente_filesystem;

// ESTRUCTURAS ADMINISTRATIVAS 
t_list* lista_global_segmentos;
t_list* tabla_segmentos_por_proceso;
t_list* tabla_huecos;
void* memoria_principal;
t_segmento* segmento_cero;
int tamanio_max_segmento_cpu;


// ------------------------------ CONFIG MEMORIA ------------------------------ //

void cargar_config_memoria(t_config* config){
    
    config_memoria.PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
    config_memoria.TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
    config_memoria.TAM_SEGMENTO_0 = config_get_int_value(config, "TAM_SEGMENTO_0");
    config_memoria.CANT_SEGMENTOS = config_get_int_value(config, "CANT_SEGMENTOS");
    config_memoria.RETARDO_MEMORIA = config_get_int_value(config, "RETARDO_MEMORIA");
    config_memoria.RETARDO_COMPACTACION = config_get_int_value(config, "RETARDO_COMPACTACION");
    config_memoria.ALGORITMO_ASIGNACION = obtener_algoritmo_asignacion(config_get_string_value(config, "ALGORITMO_ASIGNACION"));
}

t_algoritmo_asignacion obtener_algoritmo_asignacion(char* string_algoritmo){
    
    if(string_equals_ignore_case(string_algoritmo, "FIRST")){
        return FIRST;
    }
    if(string_equals_ignore_case(string_algoritmo, "BEST")){
        return BEST;
    }
    if(string_equals_ignore_case(string_algoritmo, "WORST")){
        return WORST;
    }
    
    log_error(logger, "Hubo un error con el algoritmo de asignacion");
    return -1;
}


// ------------------------------ MANEJO DE HUECOS ------------------------------ //

t_hueco* crear_hueco(int direccion_base, int tamanio){
    
    t_hueco* hueco = malloc(sizeof(t_hueco));

    hueco->direccion_base = direccion_base;
    hueco->tamanio = tamanio;

    return hueco;
}

void agregar_hueco(t_hueco* hueco){
    
    bool comparar_por_direccion_base(void* elemento1, void* elemento2) {
        t_hueco* hueco1 = (t_hueco*)elemento1;
        t_hueco* hueco2 = (t_hueco*)elemento2;
    
        return hueco1->direccion_base < hueco2->direccion_base;
    }

    // Tenemos que meter los huecos de forma ordenada a la lista para que los algoritmos hagan bien su trabajo
    list_add_sorted(tabla_huecos, hueco, comparar_por_direccion_base);
}

void crear_y_agregar_hueco(int direccion_base, int tamanio){
    t_hueco* hueco = crear_hueco(direccion_base, tamanio);
    agregar_hueco(hueco);
}

t_hueco* consolidar_huecos(t_hueco* hueco_original, t_hueco* hueco_aledanio_anterior, t_hueco* hueco_aledanio_posterior)
{
    if(hueco_aledanio_anterior != NULL){
        log_debug(logger, "Se consolidaran los huecos: Base %d - Size: %d -- Base: %d - Size: %d", hueco_original->direccion_base, hueco_original->tamanio, hueco_aledanio_anterior->direccion_base, hueco_aledanio_anterior->tamanio);

        hueco_original->direccion_base = hueco_aledanio_anterior->direccion_base;
        hueco_original->tamanio += hueco_aledanio_anterior->tamanio;        

        list_remove_element(tabla_huecos, hueco_aledanio_anterior);
        free(hueco_aledanio_anterior);
    }

    if(hueco_aledanio_posterior != NULL){
        log_debug(logger, "Se consolidaran los huecos: Base %d - Size %d -- Base: %d - Size: %d", hueco_original->direccion_base, hueco_original->tamanio, hueco_aledanio_posterior->direccion_base, hueco_aledanio_posterior->tamanio);

        hueco_original->tamanio += hueco_aledanio_posterior->tamanio;

        list_remove_element(tabla_huecos, hueco_aledanio_posterior);
        free(hueco_aledanio_posterior);
    }

    log_debug(logger, "El hueco resultante es: Base: %d - Size: %d", hueco_original->direccion_base, hueco_original->tamanio);
    
    return hueco_original;
}

void crear_y_consolidar_huecos(int direccion_base, int tamanio) {

    // Creamos el hueco
    t_hueco* hueco = crear_hueco(direccion_base, tamanio);

    // Buscamos un potenciales huecos aledanios
    t_hueco* hueco_aledanio_1 = buscar_hueco_por_final(direccion_base);
    t_hueco* hueco_aledanio_2 = buscar_hueco_por_base(direccion_base + tamanio);

    // Consolidamos los huecos -> Si no encontro huecos aledanios esta funcion retorna el hueco original
    hueco = consolidar_huecos(hueco, hueco_aledanio_1, hueco_aledanio_2);

    agregar_hueco(hueco);
}

t_hueco* buscar_hueco_por_base(int direccion_base){
    bool coincide_con_base(void* hueco){
        return ((t_hueco*)hueco)->direccion_base == direccion_base;
    }

    return ((t_hueco*)list_find(tabla_huecos, coincide_con_base));
}

t_hueco* buscar_hueco_por_final(int direccion_final){
    bool coincide_con_final(void* hueco){
        return ((t_hueco*)hueco)->direccion_base + ((t_hueco*)hueco)->tamanio == direccion_final;
    }

    return ((t_hueco*)list_find(tabla_huecos, coincide_con_final));
}


// ------------------------------ MANEJO DE SEGMENTOS ------------------------------ //

// Crea segmento, lo agrega a tabla global de segmentos y actualiza tabla de huecos.
t_segmento* crear_segmento(int id, int direccion_base, int tamanio) {
    t_segmento* segmento = malloc(sizeof(t_segmento));

    segmento->id = id;
    segmento->direccion_base = direccion_base;
    segmento->tamanio = tamanio;

    // Se agrega el segmento creado a la lista global de segmentos
    list_add(lista_global_segmentos, segmento);

    return segmento;
}


// FUNCION IMPROVISADA, ESTO ESTABA EN CREAR_SEGMENTO -> BORRAR
void acutalizar_tabla_de_huecos(t_segmento* segmento){
    // Actualizar tabla de huecos.

    t_hueco* hueco = buscar_hueco_por_base(segmento->direccion_base);

    // log_debug(logger, "Base del hueco (previo a creacion segmento) -> %d", hueco->direccion_base);
    // log_debug(logger, "Tamanio del hueco (previo a creacion segmento) -> %d", hueco->direccion_base);

    hueco->direccion_base += segmento->tamanio;
    hueco->tamanio -= segmento->tamanio;

    // log_debug(logger, "Base del hueco (posterior a creacion segmento) -> %d", hueco->direccion_base);
    // log_debug(logger, "Tamanio del hueco (posterior a creacion segmento) -> %d", hueco->tamanio);

    if(hueco->tamanio == 0){
        log_debug(logger, "Hueco eliminado \n");
        list_remove_element(tabla_huecos, hueco);
        free(hueco);
    }
}

// Agrega segmento a la tabla de segmentos por proceso
void agregar_segmento(t_segmento* segmento, int pid){
    
    // Buscar en tabla_segmentos_por_proceso por pid la lista de segmentos de un proceso
    t_tabla_proceso* tabla_proceso;
    bool coincide_con_pid(void* elemento){
        return ((t_tabla_proceso*)elemento)->pid == pid;
    }

    tabla_proceso = (t_tabla_proceso*)(list_find(tabla_segmentos_por_proceso, coincide_con_pid));

    if(tabla_proceso == NULL)
        log_error(logger,"No se encontro la tabla del proceso de PID %d", pid);

    list_add(tabla_proceso->lista_segmentos, segmento);

    log_debug(logger, "Segmento %d agregado a proceso %d: Base %d, Tamanio: %d", segmento->id, pid, segmento->direccion_base, segmento->tamanio);
}



t_segmento* buscar_segmento_por_base(int direccion_base, t_list* tabla_segmentos){
    bool coincide_con_base(void* segmento){
        return ((t_segmento*)segmento)->direccion_base == direccion_base;
    }

    return ((t_segmento*)list_find(tabla_segmentos, coincide_con_base));
}

// Este proceso se refiere al de la tabla de segmentos por proceso
t_tabla_proceso* buscar_proceso_por_pid(t_list* lista ,int pid) {
    
    t_list_iterator* lista_it = list_iterator_create(lista);

    // si lo encuentra, lo saca de la lista y lo devuelve
    while (list_iterator_has_next(lista_it)) {
        
        t_tabla_proceso* proceso = (t_tabla_proceso*)list_iterator_next(lista_it);
        if (proceso->pid == pid) {
            list_iterator_destroy(lista_it);       
            return proceso;
        }
    }
    

    list_iterator_destroy(lista_it);
    return NULL;
}

// Recibe por parametro el proceso asi en donde la llamamos tenemos idea del proceso del que se trata
t_segmento* buscar_segmento_en_tabla_por_proceso(t_list* tabla_segmentos, t_tabla_proceso** proceso, int pid, int id_segmento) {

    t_segmento* segmento;

    // Buscamos primero el proceso y luego el segmento en la tabla de ese proceso
    *proceso = buscar_proceso_por_pid(tabla_segmentos_por_proceso, pid);
    segmento = buscar_segmento_por_id(id_segmento, (*proceso)->lista_segmentos);

    return segmento;
}

t_segmento* buscar_segmento_en_tabla_global(t_list* tabla_segmentos, int direccion_base) {

    t_segmento* segmento;
    
    // Buscamos el segmento segun la base 
    segmento = buscar_segmento_por_base(direccion_base, lista_global_segmentos);

    return segmento;
}

void eliminar_segmento_de_tabla(t_list* tabla_segmentos, t_segmento* segmento, char* tipo_tabla, int pid) {

    if(string_equals_ignore_case(tipo_tabla, "GLOBAL")) {
        list_remove_element(tabla_segmentos, segmento);
        log_debug(logger, "Segmento %d removido de memoria: Base %d, Tamanio: %d", segmento->id, segmento->direccion_base, segmento->tamanio);
        free(segmento);
    }
    if(string_equals_ignore_case(tipo_tabla, "PROCESO")) {
        list_remove_element(tabla_segmentos, segmento);
        log_debug(logger, "Segmento %d removido de proceso %d: Base %d, Tamanio: %d", segmento->id, pid, segmento->direccion_base, segmento->tamanio);

        // Aca no hacemos free porque la idea es hacerlo solamente cuando eliminamos de la tabla global
        // Caso contrario, estariamos haciendo dos frees porque el segmento es el mismo en ambas tablas
    }
}

// COMPACTACION
t_segmento* mover_segmentos() {

    // Filtar a los que tengan tamaño distinto de 0

    t_list* lista_global_filtrada;

    bool tamanio_no_nulo(void* segmento){
        return ((t_segmento*)segmento)->tamanio != 0;
    }

    lista_global_filtrada = list_filter(lista_global_segmentos, tamanio_no_nulo);

    //mostrar_tabla_segmentos(lista_global_filtrada);

    t_list_iterator* lista_it = list_iterator_create(lista_global_filtrada);

    t_segmento* segmento = NULL;
    t_segmento* segmento_anterior = NULL;

    while(list_iterator_has_next(lista_it)) {
        
        segmento = (t_segmento*)list_iterator_next(lista_it);
        char* datos_leidos;

        if(segmento->direccion_base != 0) {
            
            // Copiamos en datos_leidos los datos del segmento que vamos a modificar y los liberamos
            datos_leidos = malloc(segmento->tamanio);
            memcpy(datos_leidos, memoria_principal + segmento->direccion_base, segmento->tamanio);


            //log_debug(logger, "DATOS_LEIDOS: %s", datos_leidos);

            // Movemos el segmento
            segmento->direccion_base = segmento_anterior->direccion_base + segmento_anterior->tamanio;

            // Escribimos en la nueva direccion_base del segmento los datos que tenia dicho segmento
            memcpy(memoria_principal + segmento->direccion_base, datos_leidos, segmento->tamanio);
            
            free(datos_leidos);
        }

        segmento_anterior = segmento;
    }
    list_iterator_destroy(lista_it);

    list_destroy(lista_global_filtrada);

    // Retornamos el ultimo segmento
    return segmento;
}

void mostrar_tabla_segmentos_proceso(t_list* tabla_segmentos, int pid) {
    
    log_warning(logger, "ENTRE A MOSTRAR TABLA");

    t_list_iterator* lista_it = list_iterator_create(tabla_segmentos);

    while (list_iterator_has_next(lista_it)) {
        t_segmento* segmento = (t_segmento*)list_iterator_next(lista_it);

        if(segmento->tamanio != 0)
            log_warning(logger, "PID: %d - Segmento: %d - Base: %d - Tamanio: %d \n", pid, segmento->id, segmento->direccion_base, segmento->tamanio);  //LOG RESULTADO COMPACTACION
    }
    

    list_iterator_destroy(lista_it);
}


void mostrar_resultado_compactacion(t_list* tabla_proceso) {
    
    t_list_iterator* iterador_tabla_proceso = list_iterator_create(tabla_proceso);

    while (list_iterator_has_next(iterador_tabla_proceso)) {
        t_tabla_proceso* tabla = (t_tabla_proceso*)list_iterator_next(iterador_tabla_proceso);
        
        mostrar_tabla_segmentos_proceso(tabla->lista_segmentos, tabla->pid);
    }
    
    list_iterator_destroy(iterador_tabla_proceso);
}







// ------------------------------ OTROS ------------------------------ //

int espacio_restante_memoria(){
    // Yo lo calcularia como una resta: Total_memoria - tamanio de todos los segmentos (de lista global de segmentos)
    int memoria_total = config_memoria.TAM_MEMORIA;
    int suma_tamanio_segmentos = 0;

    // Recorrer lista global de segmentos e ir sumando los tamaños. Yo haria un fold porque estoy mal de la cabeza
    void* sumar_numero_con_tamanio_segmento(void* numero, void* segmento){
        int tamanio_segmento = ((t_segmento*)segmento)->tamanio;
        *(int*)numero += tamanio_segmento;
        return numero;
    }

    list_fold(lista_global_segmentos, &suma_tamanio_segmentos, sumar_numero_con_tamanio_segmento);

    int espacio_restante = memoria_total - suma_tamanio_segmentos;

    log_debug(logger, "Espacio restante en memoria: %d", espacio_restante);

    return espacio_restante;
}


// Funcion de debug para leer el espacio de usuario y chequear que se muevan bien las cosas en memoria luego de la compactacion
void leer_memoria() {

    char* char_ptr = (char*)memoria_principal;

    // Acá elegimos hasta qué direccion de memoria queremos que lea
    for(int i = 0; i < 1220; i++) {

        if(char_ptr[i] != NULL)
            log_debug(logger, "El valor escrito en la posicion %d de memoria es: %c", i, (char)char_ptr[i]);
    }

    free(char_ptr);
}

