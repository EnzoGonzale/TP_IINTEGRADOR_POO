#include <iostream>
#include "Server.h"

int main(int argc, char* argv[]) {
    // 1. Creamos el objeto principal de la aplicación.
    Server serverApp;
    
    // 2. Ejecutamos el servidor. La clase Server se encargará de
    //    inicializar y coordinar todos los demás componentes.
    serverApp.run();

    return 0;
}