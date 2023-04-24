#include "serializacion.h"



// ------------------------------ SERIALIZACION STRINGS ------------------------------ //

/*

//serializa un solo string -> lo vamos a usar dentro de un ciclo for para serializar un solo string de un array de strings

void* serializar_string(size_t* size, char* str, int desp) {
	size_t size_str = strlen(str) + 1;
	*size = sizeof(size_t) 		//tamanio total
		+ sizeof(size_t)	//tamanio del char*
		+ size_str;		//string que llego
	size_t	size_payload = *size - sizeof(size_t);
	
	void* stream = malloc(*size);	

	memcpy(stream, &size_payload, sizeof(size_t));
	memcpy(stream+sizeof(size_t), str, size_str);
	return stream;
}


//esta funcion tambien la vamos a usar dentro de un ciclo for para deserializar cada string del array por separado

void deserializar_string(void* stream, char** str) {

	size_t size_str;
	memcpy(&size_str, stream, sizeof(size_t));

	char* r_str = malloc(size_str);
	memcpy(r_str, stream+sizeof(size_t), size_str);
	*str = r_str;
}

*/


// ------------------------------ SERIALIZACION INSTRUCCIONES ------------------------------ //

void tamanio_lista(size_t* size, t_list* lista) {

    t_list_iterator* lista_it = list_iterator_create(lista);
    for(int i = 0; list_iterator_has_next(lista_it); i++) {
        *size += strlen(list_iterator_next(lista_it) + 1);
    }
    list_iterator_destroy(lista);
}




void* serializar_lista_instrucciones(size_t* size, t_list* lista_instrucciones) {

    size_t tamanio_l;

    tamanio_lista(&tamanio_l, lista_instrucciones);

    printf("%d", tamanio_l);

}


void* serializar_instrucciones(size_t* size, t_list* instrucciones) {
       
    // stream lista de instrucciones

    size_t size_instrucciones;
    
    void* stream_instrucciones = serializar_lista_instrucciones(&size_instrucciones, instrucciones);

    
    // stream completo

    //size_t size_total =  sizeof(op_code) 
     //                   + sizeof(size_t) // size total del stream
     //                   + sizeof(size_t) // size instrucciones
     //                   + size_instrucciones;   //instrucciones
    
   
   //void* stream = malloc(size_total);

    //op_code cop = INSTRUCCIONES;

   
   /* size_t size_payload = *size - sizeof(op_code) - sizeof(size_t);

    void* stream = malloc(*size);

    op_code cop = INSTRUCCIONES;
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &size_instrucciones, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t)*2 , instrucciones, size_instrucciones);
      
      */
    //return stream; 

    //free(stream_instrucciones);
    //*size = size_total;
    //return stream;       
}


void deserializar_instrucciones(void* stream, char*** instrucciones) {

    //size_t size_instrucciones;
    //memcpy(&size_instrucciones, stream, sizeof(size_t));

    /*char** arr = malloc((size_instrucciones + 1) * sizeof(char*));

    for(int i = 0; i < size_instrucciones; i++)
    {
        arr[i] = read_string(buffer, desp);
    }
    arr[size_instrucciones] = NULL;*/
    
    // ESTAMOS PASANDO MAL EL ARRAY DE STRINGS ???????
    
    //char** instrucciones_r = malloc(size_instrucciones);


   // memcpy(instrucciones_r, stream+sizeof(size_t), size_instrucciones);
    
    //*instrucciones = instrucciones_r;
}



// ------------------------------ SERIALIZACION NUMERO ------------------------------ //

/*

void* serializar_numero(int numero) {
    
    //creamos un stream intermedio que guarde el codigo de operacion y el mensaje
    void* stream = malloc(sizeof(op_code) + sizeof(int));
    
    op_code codigo = NUMERO;

    //copiamos el codigo de operacion en el stream y despues nos corremos y copiamos el mensaje (payload)
    memcpy(stream, &codigo, sizeof(op_code));
    memcpy(stream+sizeof(op_code), &numero, sizeof(numero));
    
    // Aca retorna un stream con el codigo de operación al principio y concatendo a esto está nuestro dato (un int en este caso).
    return stream;
}


void deserializar_numero(void* stream, int* numero) {
    
    //aca estamos copiando el stream en la variable numero -> se recibe el mensaje
    memcpy(numero, stream, sizeof(int));                
}

*/









