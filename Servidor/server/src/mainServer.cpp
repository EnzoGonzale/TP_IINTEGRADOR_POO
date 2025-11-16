#include "Server.h"
#include <iostream>
#include "ServiceLocator.h"
#include "SerialComunicator.h"

int main(int argc, char* argv[]) {
    // 1. Creamos el objeto principal de la aplicación.
    Server serverApp;
    

    // Creamos el comunicador real y lo registramos en el ServiceLocator.
    ComunicatorPort::SerialComunicator realCommunicator;
    ServiceLocator::provide(&realCommunicator);


    // 2. Ejecutamos el servidor. La clase Server se encargará de
    //    inicializar y coordinar todos los demás componentes.
    serverApp.run();

    return 0;
}