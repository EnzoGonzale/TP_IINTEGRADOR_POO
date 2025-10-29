#include "CLIHandler.h"
#include <iostream>
#include <xmlrpc-c/client_simple.hpp> // Para el cliente RPC

int main(int argc, char* argv[]) {
    std::string serverIp = "localhost"; // Por defecto, se conecta a la misma máquina.

    // Si el usuario proporciona una IP como argumento, la usamos.
    if (argc > 1) {
        serverIp = argv[1];
    }
    std::string serverUrl = "http://" + serverIp + ":8080/RPC2";

    // 1. Creamos una instancia del cliente XML-RPC.
    xmlrpc_c::clientSimple rpcClient;

    // 2. Creamos el manejador de la línea de comandos, pasándole el cliente RPC.
    CLIHandlerNamespace::CLIHandler cliHandler(rpcClient, serverUrl);

    // 3. Iniciamos el bucle interactivo del cliente.
    cliHandler.start();

    return 0;
}