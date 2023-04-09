#include "init.h"
#include "sockets.h"
#include <commons/log.h>
#include <commons/config.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <commons/collections/list.h>

// Así en los .h de cada módulo solo ponemos include shared.h
// Capaz no está muy bien porque puede haber una biblioteca que en un módulo no se use. Después vemos en qué queda.