
#ifndef CLIHANDLER_H
#define CLIHANDLER_H

#include <string>
#include "User.h"
#include <optional>
#include <vector>
// Incluimos las cabeceras del cliente XML-RPC
#include <xmlrpc-c/client_simple.hpp>

namespace CLIHandlerNamespace
{


/// 
/// class CLIHandler

class CLIHandler
{
public:
  // Constructors/Destructors  

  /// @brief Constructor para el cliente CLI.
  /// @param rpcClient El cliente XML-RPC que se usará para las llamadas.
  /// @param serverUrl La URL del servidor RPC.
  CLIHandler(xmlrpc_c::clientSimple& rpcClient, const std::string& serverUrl);

  /// 
  /// Empty Destructor
  virtual ~CLIHandler();



  /// @brief Inicia el ciclo de vida del Command Line Interface.
  void start();

private:
  /// @brief Muestra el menú principal de opciones al usuario.
  void displayMenuAdmin();
  void displayMenuOperator();


  /// @brief Procesa un comando ingresado por el usuario.
  /// @param command El comando a procesar.
  void processCommand(const std::string& command);

  /// @brief Gestiona el bucle de inicio de sesión.
  /// @return True si el inicio de sesión fue exitoso, false en caso contrario.
  bool login();

  /// @brief Inicia el bucle principal de la aplicación después del login.
  void mainLoop();

private:
  // Usamos std::optional para representar que puede haber o no un usuario logueado.
  // Ahora almacenamos las credenciales para enviarlas en cada llamada.
  std::optional<std::tuple<std::string, std::string, UserRole>> currentUserInfo;

  xmlrpc_c::clientSimple& rpcClient;
  std::string serverUrl;

  // --- Atributos para el modo de aprendizaje ---
  bool learningModeActive = false;
  std::string learningTaskId;
  std::string learningTaskName;
  std::vector<std::string> learnedGCodeCommands;

};

} // namespace CLIHandlerNamespace

#endif // CLIHANDLER_H
