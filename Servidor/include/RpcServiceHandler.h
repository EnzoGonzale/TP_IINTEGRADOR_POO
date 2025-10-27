
#ifndef RPCSERVICEHANDLER_H
#define RPCSERVICEHANDLER_H

#include <string>
#include "Robot.h"
#include "ReportGenerator.h"
#include "TaskManager.h"
#include "AuthenticationService.h"

#include <xmlrpc-c/registry.hpp>



namespace RpcServiceHandlerNamespace
{

/// 
/// class RpcServiceHandler

class RpcServiceHandler
{
public:
  // Constructors/Destructors  
  RpcServiceHandler(
    AuthenticationServiceNamespace::AuthenticationService& authService,
    RobotNamespace::Robot& robot,
    TaskManager& taskManager
  );

  /// 
  /// Empty Destructor
  virtual ~RpcServiceHandler();

  /// @brief Registra todos los métodos RPC en el registro proporcionado.
  /// @param registry El registro del servidor XML-RPC.
  void registerMethods(xmlrpc_c::registry &registry);

private:
  AuthenticationServiceNamespace::AuthenticationService& authService;
  RobotNamespace::Robot& robot;
  TaskManager& taskManager;
  ReportGenerator reportGenerator;

};

} // namespace RpcServiceHandlerNamespace

#endif // RPCSERVICEHANDLER_H
