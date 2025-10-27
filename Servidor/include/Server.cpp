#include "Server.h"
#include <iostream>
#include <chrono> // Para std::this_thread::sleep_for
#include <xmlrpc-c/server_abyss.hpp>
#include <string>
#include <filesystem> // Requerido para C++17

// Helper para obtener la ruta del directorio del proyecto
std::string getProjectDirectory() {
    // __FILE__ es una macro que se expande a la ruta completa de este archivo de código fuente.
    std::filesystem::path source_path = __FILE__;
    // Subimos dos niveles desde /include/Server.cpp para llegar a la raíz del proyecto.
    return source_path.parent_path().parent_path().string();
}

// Constructors/Destructors

Server::Server() 
    : running(false),
      dbManager(getProjectDirectory() + "/server_database.db"),
      authService(dbManager),
      robot(),
      reportGenerator(), // Se mantiene por si los métodos RPC la necesitan
      taskManager("./tasks.json"),
      rpcHandler(authService, robot, taskManager)
{ 
    // Cargamos las tareas al iniciar el servidor.
    if (taskManager.loadTasks()) {
        std::cout << "[Server] Tareas cargadas exitosamente desde tasks.json." << std::endl;
    } else {
        std::cout << "[Server] Error al cargar las tareas desde tasks.json." << std::endl;
    }
}

Server::~Server()
{
}

void Server::startRpcServer() {
    try {
        xmlrpc_c::registry myRegistry;
        rpcHandler.registerMethods(myRegistry);

        xmlrpc_c::serverAbyss myAbyssServer(
            xmlrpc_c::serverAbyss::constrOpt()
            .registryP(&myRegistry)
            .portNumber(8080));
        
        std::cout << "[RPC Server] Servidor XML-RPC iniciado en el puerto 8080. Esperando peticiones..." << std::endl;
        myAbyssServer.run(); // Esto bloquea este hilo y empieza a escuchar.

    } catch (std::exception const& e) {
        std::cerr << "[RPC Server] Algo salió mal: " << e.what() << std::endl;
    }
}

void Server::run()
{
    // El servidor ahora solo inicia el RPC Server y bloquea el hilo principal.
    std::cout << "[Main] Iniciando Servidor RPC. Use Ctrl+C para detener." << std::endl;
    startRpcServer(); // Esta llamada es bloqueante.
    // Cuando startRpcServer() termina (por ejemplo, por Ctrl+C), el programa finaliza.
}

void Server::shutdown()
{
    // Lógica para detener los servicios de forma segura.
}
