#include "serializacion.h"


// ------------------------------ SERIALIZACION ARRAY STRINGS ------------------------------ //

void* serializar_array_strings(size_t* size, char** array_strings) {
    
    int cant_elementos_array = string_array_size(array_strings); 
   
    // ESTO ES EL STREAM QUE DEVUELVE ESTA FUNCIÓN
    // size_array + size_string1 + string1 + size_string2 + string2...
    //              --------------------------------------------------
    //                   de esto se encarga serializar_string    

    
    size_t size_array = 0;
    
    
    char** arr = malloc((cant_elementos_array + 1) * sizeof(char*));

    //Primero tenemos que leer el tamaño del string

    //printf("%d", size_instrucciones);

    for(int i = 0; i < cant_elementos_array; i++) {
        // Sumar a size_array el size de cada string serializado
        arr[i] = serializar_string();
    }

    return arr;

    // 
    
}

// serializar string, deberia dar sizeString + string. Capaz podemos tener una lista de esa estructura.



// ------------------------------ SERIALIZACION STRINGS ------------------------------ //

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


// ------------------------------ SERIALIZACION INSTRUCCIONES ------------------------------ //

void* serializar_instrucciones(size_t* size, char** instrucciones) {
    
    *size = sizeof(op_code);

    /*
    Objetivo: Devolver Paquete = Codigo de Operacion + Payload (streamArray)
    Pero no solo debemos devolver eso, sino que debemos modificar la variable size tal que contenga el size del paquete.

    

    
    */

    
    serializar_array_strings(size, instrucciones); // Obtenemos size total en "size".

    
    // antes de guardar las cosas en el stream tenemos que saber el tamaño de todo




    /*
    
    
    *size = sizeof(op_code) 
            + sizeof(size_t) // size total
            
            + sizeof(size_t) // size instrucciones
            + size_instrucciones;
    
    size_t size_payload = *size - sizeof(op_code) - sizeof(size_t);

    void* stream = malloc(*size);

    op_code cop = INSTRUCCIONES;
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &size_instrucciones, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t)*2 , instrucciones, size_instrucciones);
      
      */
    return stream;        
}


void deserializar_instrucciones(void* stream, char*** instrucciones) {

    size_t size_instrucciones;
    memcpy(&size_instrucciones, stream, sizeof(size_t));

    /*char** arr = malloc((size_instrucciones + 1) * sizeof(char*));

    for(int i = 0; i < size_instrucciones; i++)
    {
        arr[i] = read_string(buffer, desp);
    }
    arr[size_instrucciones] = NULL;*/
    
    // ESTAMOS PASANDO MAL EL ARRAY DE STRINGS ???????
    
    //char** instrucciones_r = malloc(size_instrucciones);


    memcpy(instrucciones_r, stream+sizeof(size_t), size_instrucciones);
    
    *instrucciones = instrucciones_r;
}



// ------------------------------ SERIALIZACION NUMERO ------------------------------ //

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










