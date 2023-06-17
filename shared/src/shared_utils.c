#include "shared_utils.h"

// Este archivo está compuesto por funciones auxiliares, para que no ocupen espacio en otros archivos más específicos.

t_dictionary* diccionario_instrucciones;
t_dictionary* diccionario_registros_cpu;

void* sumarSizeConLongitudString(void* a, void* b){
    size_t y = strlen((char*)b) + 1;
    *(size_t*)a += y;
    return a;
}

size_t tamanio_lista(t_list* lista){
    size_t tamanio_lista = 0;
    list_fold(lista, &tamanio_lista, sumarSizeConLongitudString); // Devuelve lo foldeado pero hice que modificara tamanio_lista y listo, que al principio es la seed.
    return tamanio_lista;
}


char* lista_a_string(t_list* lista, char string[]) {
    
    string[0] = '\0';

    t_list_iterator* lista_it = list_iterator_create(lista);
    for(int i = 0; list_iterator_has_next(lista_it); i++) {
       strcat(string, (char*)list_iterator_next(lista_it));
       if(list_size(lista) > 1 && list_iterator_has_next(lista_it))
        strcat(string, ", ");
    }
    list_iterator_destroy(lista_it);

    return string;

}

char* lista_pids_a_string(t_list* lista, char string[]) {
    
    string[0] = '\0';

    t_list_iterator* lista_it = list_iterator_create(lista);
    for(int i = 0; list_iterator_has_next(lista_it); i++) {
        char* tmp = string_itoa(*((int*)list_iterator_next(lista_it)));
       strcat(string, tmp);
       free(tmp);
       if(list_size(lista) > 1 && list_iterator_has_next(lista_it))
        strcat(string, ", ");
    }
    list_iterator_destroy(lista_it);

    return string;

}



void mostrar_lista(t_list* lista) {
    
    printf("Mostrando lista: \n");
    t_list_iterator* lista_it = list_iterator_create(lista);
    for(int i = 0; list_iterator_has_next(lista_it); i++) {
        printf("%s \n", (char*)list_iterator_next(lista_it));
    }
    list_iterator_destroy(lista_it);
}

void copiar_stream_en_variable_y_desplazar(void* variable, void* stream, size_t tamanio_elemento, size_t* desplazamiento){
    
    memcpy(variable, stream + *desplazamiento, tamanio_elemento);
    *desplazamiento += tamanio_elemento;
}


void copiar_variable_en_stream_y_desplazar(void* paquete, void* elemento, size_t tamanio_elemento, size_t* desplazamiento){
    
    memcpy(paquete + *desplazamiento, elemento, tamanio_elemento);
    *desplazamiento += tamanio_elemento;
}


void copiar_en_stream_y_desplazar_lista_strings_con_tamanios(void* paquete, t_list* lista_instrucciones){
    
    // El vscode se enoja con esto pero funca igual.
    // La definí como función auxiliar para poder usar al paquete
    // Me gustaria mandar el paquete como parámetro pero list_iterate admite funciones con 1 solo parámetro.
    size_t desplazamiento = 0;
    
    void copiar_y_desplazar_string_con_tamanio(void* string) {
        size_t tamanio = strlen((char*)string) + 1;
        copiar_variable_en_stream_y_desplazar(paquete, &tamanio, sizeof(size_t), &desplazamiento);
        copiar_variable_en_stream_y_desplazar(paquete, string, tamanio, &desplazamiento);
    }

    list_iterate(lista_instrucciones, copiar_y_desplazar_string_con_tamanio);
}


void copiar_en_stream_y_desplazar_tabla_segmentos(void* paquete, t_list* tabla_segmentos){
    
    // El vscode se enoja con esto pero funca igual.
    // La definí como función auxiliar para poder usar al paquete
    // Me gustaria mandar el paquete como parámetro pero list_iterate admite funciones con 1 solo parámetro.
    size_t desplazamiento = 0;
    
    void copiar_y_desplazar_segmento_con_tamanio(void* segmento) {
        size_t tamanio = 12;        //SI NO ES T_SEGMENTO ES SEGMENTO  //ESTA HARDCODEADO
        copiar_variable_en_stream_y_desplazar(paquete, &tamanio, sizeof(size_t), &desplazamiento);
        copiar_variable_en_stream_y_desplazar(paquete, segmento, tamanio, &desplazamiento);
    }

    list_iterate(tabla_segmentos, copiar_y_desplazar_segmento_con_tamanio);
}



void asignar_a_registro(char* registro, char* valor, t_registros_cpu* registros){ // CUIDADO, SI EL VALOR ES MAS GRANDE QUE EL REGISTRO TE SOBREESCRIBE LOS OTROS.
    registro_cpu reg = (intptr_t) dictionary_get(diccionario_registros_cpu, registro);

    char* registro_objetivo;

    int longitud;

    switch(reg) {
        case AX: 
            registro_objetivo = registros->AX;
            longitud = 4;
            break;
        case BX: 
            registro_objetivo = registros->BX;
            longitud = 4;
            break;
        case CX: 
            registro_objetivo = registros->CX;
            longitud = 4;
            break;
        case DX: 
            registro_objetivo = registros->DX;
            longitud = 4;
            break;
        case EAX: 
            registro_objetivo = registros->EAX;
            longitud = 8;
            break;
        case EBX: 
            registro_objetivo = registros->EBX;
            longitud = 8;
            break;
        case ECX: 
            registro_objetivo = registros->ECX;
            longitud = 8;
            break;
        case EDX: 
            registro_objetivo = registros->EDX;
            longitud = 8;
            break;
        case RAX: 
            registro_objetivo = registros->RAX;
            longitud = 16;
            break;
        case RBX: 
            registro_objetivo = registros->RBX;
            longitud = 16;
            break;
        case RCX: 
            registro_objetivo = registros->RCX;
            longitud = 16;
            break;
        case RDX: 
            registro_objetivo = registros->RDX;
            longitud = 16;
            break;
        default:
            printf("ERROR: EL REGISTRO NO EXISTE !!! \n");
    }

    strncpy(registro_objetivo, valor, longitud);
}

//TODO
char* leer_de_registro(char* registro, t_registros_cpu* registros){
    registro_cpu reg = (intptr_t) dictionary_get(diccionario_registros_cpu, registro);

    char* registro_objetivo;
    int longitud;

    switch(reg) {
        case AX: 
            registro_objetivo = registros->AX;
            longitud = 4;
            break;
        case BX: 
            registro_objetivo = registros->BX;
            longitud = 4;
            break;
        case CX: 
            registro_objetivo = registros->CX;
            longitud = 4;
            break;
        case DX: 
            registro_objetivo = registros->DX;
            longitud = 4;
            break;
        case EAX: 
            registro_objetivo = registros->EAX;
            longitud = 8;
            break;
        case EBX: 
            registro_objetivo = registros->EBX;
            longitud = 8;
            break;
        case ECX: 
            registro_objetivo = registros->ECX;
            longitud = 8;
            break;
        case EDX: 
            registro_objetivo = registros->EDX;
            longitud = 8;
            break;
        case RAX: 
            registro_objetivo = registros->RAX;
            longitud = 16;
            break;
        case RBX: 
            registro_objetivo = registros->RBX;
            longitud = 16;
            break;
        case RCX: 
            registro_objetivo = registros->RCX;
            longitud = 16;
            break;
        case RDX: 
            registro_objetivo = registros->RDX;
            longitud = 16;
            break;
        default:
            printf("ERROR: EL REGISTRO NO EXISTE !!! \n");
    }

    char valor[longitud];

    strncpy(valor, registro_objetivo, longitud);

    return valor;
}

void inicializar_diccionarios() {
    
    // Diccionario de instrucciones

    diccionario_instrucciones = dictionary_create();

    dictionary_put(diccionario_instrucciones, "SET", (void*) (intptr_t) SET); 
    dictionary_put(diccionario_instrucciones, "MOV_IN", (void*) (intptr_t) MOV_IN);
    dictionary_put(diccionario_instrucciones, "MOV_OUT", (void*) (intptr_t) MOV_OUT);
    dictionary_put(diccionario_instrucciones, "I/O", (void*) (intptr_t) IO);
    dictionary_put(diccionario_instrucciones, "F_OPEN", (void*) (intptr_t) F_OPEN);
    dictionary_put(diccionario_instrucciones, "F_CLOSE", (void*) (intptr_t) F_CLOSE);
    dictionary_put(diccionario_instrucciones, "F_SEEK", (void*) (intptr_t) F_SEEK);
    dictionary_put(diccionario_instrucciones, "F_READ", (void*) (intptr_t) F_READ);
    dictionary_put(diccionario_instrucciones, "F_WRITE",(void*) (intptr_t) F_WRITE);
    dictionary_put(diccionario_instrucciones, "F_TRUNCATE",(void*) (intptr_t) F_TRUNCATE);
    dictionary_put(diccionario_instrucciones, "WAIT",(void*) (intptr_t) WAIT);
    dictionary_put(diccionario_instrucciones, "SIGNAL",(void*) (intptr_t) SIGNAL);
    dictionary_put(diccionario_instrucciones, "CREATE_SEGMENT",(void*) (intptr_t) CREATE_SEGMENT);
    dictionary_put(diccionario_instrucciones, "DELETE_SEGMENT",(void*) (intptr_t) DELETE_SEGMENT);
    dictionary_put(diccionario_instrucciones, "YIELD",(void*) (intptr_t) YIELD);
    dictionary_put(diccionario_instrucciones, "EXIT",(void*) (intptr_t) EXIT); 

    // Diccionario de registros CPU
    diccionario_registros_cpu = dictionary_create();

    dictionary_put(diccionario_registros_cpu, "AX",  (void*) (intptr_t) AX);
    dictionary_put(diccionario_registros_cpu, "BX",  (void*) (intptr_t) BX);
    dictionary_put(diccionario_registros_cpu, "CX",  (void*) (intptr_t) CX);
    dictionary_put(diccionario_registros_cpu, "DX",  (void*) (intptr_t) DX);
    dictionary_put(diccionario_registros_cpu, "EAX",  (void*) (intptr_t) EAX);
    dictionary_put(diccionario_registros_cpu, "EBX",  (void*) (intptr_t) EBX);
    dictionary_put(diccionario_registros_cpu, "ECX",  (void*) (intptr_t) ECX);
    dictionary_put(diccionario_registros_cpu, "EDX",  (void*) (intptr_t) EDX);
    dictionary_put(diccionario_registros_cpu, "RAX",  (void*) (intptr_t) RAX);
    dictionary_put(diccionario_registros_cpu, "RBX",  (void*) (intptr_t) RBX);
    dictionary_put(diccionario_registros_cpu, "RCX",  (void*) (intptr_t) RCX);
    dictionary_put(diccionario_registros_cpu, "RDX",  (void*) (intptr_t) RDX);
}


// Crear en memoria estructuras y devolver contexto virgen.
t_contexto_ejecucion* crear_contexto(){
    t_contexto_ejecucion* contexto = malloc(sizeof(t_contexto_ejecucion));
    contexto->instrucciones = list_create();
    contexto->registros_cpu = malloc(sizeof(t_registros_cpu));

    return contexto;
}

void registros_copypaste(t_registros_cpu* registros_destino, t_registros_cpu* registros_origen){
    asignar_a_registro("AX", registros_origen->AX, registros_destino);
    asignar_a_registro("BX", registros_origen->BX, registros_destino);
    asignar_a_registro("CX", registros_origen->CX, registros_destino);
    asignar_a_registro("DX", registros_origen->DX, registros_destino);

    asignar_a_registro("EAX", registros_origen->EAX, registros_destino);
    asignar_a_registro("EBX", registros_origen->EBX, registros_destino);
    asignar_a_registro("ECX", registros_origen->ECX, registros_destino);
    asignar_a_registro("EDX", registros_origen->EDX, registros_destino);
    
    asignar_a_registro("RAX", registros_origen->RAX, registros_destino);
    asignar_a_registro("RBX", registros_origen->RBX, registros_destino);
    asignar_a_registro("RCX", registros_origen->RCX, registros_destino);
    asignar_a_registro("RDX", registros_origen->RDX, registros_destino);
}

// Jere podrías haber usado list_add_all de las commons !!
// NO
// Yo queria duplicar los elementos, no queria hacer que ambas listas apunten a los mismos elementos.
// Mas que nada porque para mi el contexto de ejecucion no deberia apuntar a campos de un proceso, sino que deberia tener su propia estructura.
void lista_copypaste(t_list* lista_objetivo, t_list* lista_origen) {
    t_list_iterator* lista_it = list_iterator_create(lista_origen);

    while (list_iterator_has_next(lista_it)) {
        char* instruccion = strdup((char*)list_iterator_next(lista_it));
        list_add(lista_objetivo, instruccion);
    }
    
    list_iterator_destroy(lista_it);
}

void tabla_copypaste(t_list* lista_objetivo, t_list* lista_origen) {
    t_list_iterator* lista_it = list_iterator_create(lista_origen);

    while (list_iterator_has_next(lista_it)) {
        t_segmento* segmento = (t_segmento*)list_iterator_next(lista_it);
        list_add(lista_objetivo, segmento);
    }
    
    list_iterator_destroy(lista_it);
}



// Dado un proceso se carga un contexto, primero crea el contexto. Siempre que se carga hay una creacion previa.
t_contexto_ejecucion* cargar_contexto(t_pcb* proceso){
    t_contexto_ejecucion* contexto = crear_contexto();

    contexto->pid = proceso->pid;
    contexto->pc = proceso->pc;
    contexto->tabla_segmentos = proceso->tabla_segmentos; //esto lo hago asi pero mepa que hay que hacer un copypaste como con los registros
    
    // No hago un = para estas dos ultimas porque la idea es que no apunten al mismo lugar, sino que solo tengan la misma informacion.
    registros_copypaste(contexto->registros_cpu, proceso->registros_cpu); 
    lista_copypaste(contexto->instrucciones, proceso->instrucciones);
    //tabla_copypaste(contexto->tabla_segmentos, proceso->tabla_segmentos);

    
    return contexto;
}

void liberar_contexto(t_contexto_ejecucion* contexto){
    free(contexto->registros_cpu);
    list_destroy_and_destroy_elements(contexto->instrucciones, free);
    free(contexto);
}

void liberar_proceso(t_pcb* proceso){
    free(proceso->registros_cpu);
    list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
    list_destroy_and_destroy_elements(proceso->instrucciones, free);
    list_destroy_and_destroy_elements(proceso->tabla_archivos_abiertos, free);
    free(proceso);
}


