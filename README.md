# tp-2023-1c-PeladOS
## Guia de deployment

### Distribución de máquinas

Se usarán 4 computadoras y los módulos se distribuirán de la siguiente forma:

- Computadora 1: Consola y CPU
- Computadora 2: Kernel
- Computadora 3: Memoria
- Computadora 4: File System

En cada computadora vamos a conectar un PuTTY (en el caso de la Computadora 1 debemos duplicar la terminal del PuTTY) y vamos a hacer un git clone del repo en cada una de ellas.

Importante tener a mano el token de GitHub en cada una de las computadoras, así como el link del repo. Estas dos cosas las tendremos en un notepad que enviaremos al mail de uno de los integrantes.

Podemos usar una página como Udrop (https://www.udrop.com/) para, una vez logueado en una de las máquinas, descargarnos el notepad y mandarlo por link a las otras computadoras, y asi evitamos tener que loguearnos en todas las computadoras para acceder al mail.


### Cambio de IPs

1. Se deben averiguar las IPs de cada computadora con el comando `ifconfig`
2. Debemos anotarnos en un notepad las IPs correspondientes a cada módulo
3. Ejecutamos el script para cambiar las IPs: `./set_ips.sh IP_CPU IP_MEMORIA IP_FILESYSTEM IP_KERNEL` (las IPs se pasan por parámetros en el orden indicado y separadas por un espacio)
4. Repetir el paso 3 por cada computadora donde hayamos clonado el repo

Hecho esto, tendremos todas las IPs cambiadas correctamente en todos los .config de todas las pruebas.

 ### Levantamiento de los módulos 

 Para levantar los módulos por cada prueba, se debe ejecutar el comando `./nombre_modulo "../tests/NOMBRE_PRUEBA/nombre_modulo.config"`

 En el caso particular de Consola, no es necesario ejecutar el comando, ya que tenemos scripts que ejecutan las instrucciones provistas por la cátedra. Entonces sólo debemos hacer `cd scripts` y luego ejecutar el script deseado con `./NOMBRE_PRUEBA.sh`

 **NOTA:** Recordar hacer `make` antes, ya que el comando mencionado no compila cada módulo automáticamente.

 **Correr con Valgrind:** Las pruebas no se corren con Valgrind en el laboratorio, pero si deseáramos chequear los leaks en cada prueba debemos ejecutar el comando `valgrind --tool=memcheck --leak-check=yes --show-possibly-lost=no --show-reachable=no --num-callers=20 ./nombre_modulo "../tests/NOMBRE_PRUEBA/nombre_modulo.config"`

 

 

  
