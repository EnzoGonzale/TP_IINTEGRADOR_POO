
#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <thread> // Para std::thread
#include "Robot.h"
#include "CLIHandler.h"
#include "RpcServiceHandler.h"
#include "AuthenticationService.h" // El servidor es dueño de los servicios
#include "DatabaseManager.h"
#include "ReportGenerator.h"

/// 
/// class Server

class Server
{
public:
  // Constructors/Destructors  



  /// 
  /// Empty Constructor
  Server();

  /// 
  /// Empty Destructor
  virtual ~Server();



  /// 
  void run();


  /// 
  void shutdown();

private:
  // Private attributes  

  /// @brief Inicia el servidor XML-RPC en un hilo separado.
  void startRpcServer();



  bool running;

  // --- Capa de Dominio/Núcleo (El servidor es dueño de estos objetos) ---
  DatabaseManagerNamespace::DatabaseManager dbManager;
  AuthenticationServiceNamespace::AuthenticationService authService;

  RobotNamespace::Robot robot;
  ReportGenerator reportGenerator; // Se mantiene por si los métodos RPC la necesitan

  // --- Capa de Aplicación/Interfaces (Servidor RPC) ---
  RpcServiceHandlerNamespace::RpcServiceHandler rpcHandler;

};

#endif // SERVER_H
