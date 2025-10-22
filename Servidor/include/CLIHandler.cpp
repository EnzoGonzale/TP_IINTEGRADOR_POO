#include "CLIHandler.h"
#include <iostream>
#include <string>
#include <limits> // Para std::numeric_limits
#include <sstream> // Para std::stringstream
#include "Logger.h" // Incluimos el Logger
#include <xmlrpc-c/base.hpp> // Para xmlrpc_c::value
#include <xmlrpc-c/client.hpp> // Para xmlrpc_c::client_error

// Constructors/Destructors


CLIHandlerNamespace::CLIHandler::CLIHandler(
    xmlrpc_c::clientSimple& rpcClient,
    const std::string& serverUrl
) : currentUserInfo(std::nullopt), 
    rpcClient(rpcClient), 
    serverUrl(serverUrl) {}

CLIHandlerNamespace::CLIHandler::~CLIHandler()
{
}

void CLIHandlerNamespace::CLIHandler::start() {
    std::cout << "=== Cliente de Control del Robot ===" << std::endl;
    std::cout << "Intentando conectar con el servidor en " << serverUrl << "..." << std::endl;
    if (login()) {
        mainLoop();
    }
    std::cout << "Saliendo del cliente..." << std::endl;
}

bool CLIHandlerNamespace::CLIHandler::login() {
    std::string username, password;
    std::cout << "Usuario: ";
    std::cin >> username;
    std::cout << "Clave: ";
    std::cin >> password;
    // Limpiamos el buffer de entrada para evitar problemas con getline más adelante.
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    try {
        xmlrpc_c::value result;
        // Usamos el método 'robot.authenticate' que ya existe en el servidor.
        rpcClient.call(serverUrl, "robot.authenticate", "ss", &result, username.c_str(), password.c_str());
        
        // Parseamos la respuesta del struct
        // Usamos inicialización con {} para evitar el "Most Vexing Parse"
        std::map<std::string, xmlrpc_c::value> const resultMap{xmlrpc_c::value_struct(result)};
        bool isAuthenticated = xmlrpc_c::value_boolean(resultMap.at("authenticated"));

        if (isAuthenticated) {
            int roleInt = xmlrpc_c::value_int(resultMap.at("role"));
            UserRole role = static_cast<UserRole>(roleInt);
            std::string roleStr = (role == UserRole::ADMIN) ? "Administrador" : "Operador";
            std::cout << "¡Inicio de sesión exitoso! Bienvenido, " << username << " (" << roleStr << ")." << std::endl;
            // Guardamos las credenciales y el rol para usarlas en futuras llamadas.
            currentUserInfo = std::make_tuple(username, password, role);
            return true;
        } else {
            std::cout << "Error: Usuario o clave incorrectos." << std::endl;
            currentUserInfo.reset();
            return false;
        }
    } catch (const girerr::error& e) {
        std::cerr << "[CLI] Error de conexión con el servidor: " << e.what() << std::endl;
        std::cerr << "Asegúrese de que el servidor esté corriendo ('make run')." << std::endl;
        return false;
    } catch (const xmlrpc_c::fault& f) {
        std::cerr << "[CLI] Error de autenticación RPC: " << f.getDescription() << std::endl;
        return false;
    }
}

void CLIHandlerNamespace::CLIHandler::mainLoop() {
    std::string command;
    
    while (true) {
        // Llamamos al menú correcto según el rol del usuario
        if (currentUserInfo && std::get<2>(*currentUserInfo) == UserRole::ADMIN) {
            displayMenuAdmin();
        } else {
            displayMenuOperator();
        }

        std::cout << "> ";
        std::getline(std::cin, command);

        if (std::cin.eof() || command.empty()) continue;
        if (command == "salir") {
            break;
        }
        processCommand(command);
    }
}

void CLIHandlerNamespace::CLIHandler::displayMenuAdmin() {
    std::cout << "\n+--------------------------------------------------------------------------------+" << std::endl;
    std::cout << "|                      PANEL DE CONTROL DEL ROBOT                                |" << std::endl;
    std::cout << "+--------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Conexión:                                                          |" << std::endl;
    std::cout << "|   conectar                  - Conecta con el robot.                            |" << std::endl;
    std::cout << "|   desconectar               - Desconecta el robot.                             |" << std::endl;
    std::cout << "+--------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Usuario:                                                           |" << std::endl;
    std::cout << "|   user_add <user> <pass> <role> - Añade un usuario (role: 0=ADMIN, 1=OPERATOR) |" << std::endl;
    std::cout << "+--------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Control:                                                           |" << std::endl;
    std::cout << "|   motores_on                - Activa los motores.                              |" << std::endl;
    std::cout << "|   motores_off               - Desactiva los motores.                           |" << std::endl;
    std::cout << "|   mover <x> <y> <z> <vel>   - Mueve el efector a una posición.                 |" << std::endl;
    std::cout << "|   efector_on                - Activa el efector final.                         |" << std::endl;
    std::cout << "|   efector_off               - Desactiva el efector final.                      |" << std::endl;
    std::cout << "+--------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Información y Salida:                                              |" << std::endl;
    std::cout << "|   estado                    - Muestra el estado actual del robot.              |" << std::endl;
    std::cout << "|   salir                     - Cierra la consola.                               |" << std::endl;
    std::cout << "+--------------------------------------------------------------------------------+" << std::endl;
}

void CLIHandlerNamespace::CLIHandler::displayMenuOperator() {
    std::cout << "\n+--------------------------------------------------------------------------------+" << std::endl;
    std::cout << "|                      PANEL DE CONTROL DEL ROBOT                                |" << std::endl;
    std::cout << "+--------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Control:                                                           |" << std::endl;
    std::cout << "|   motores_on                - Activa los motores.                              |" << std::endl;
    std::cout << "|   motores_off               - Desactiva los motores.                           |" << std::endl;
    std::cout << "|   mover <x> <y> <z> <vel>   - Mueve el efector a una posición.                 |" << std::endl;
    std::cout << "|   efector_on                - Activa el efector final.                         |" << std::endl;
    std::cout << "|   efector_off               - Desactiva el efector final.                      |" << std::endl;
    std::cout << "+--------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Información y Salida:                                              |" << std::endl;
    std::cout << "|   estado                    - Muestra el estado actual del robot.              |" << std::endl;
    std::cout << "|   salir                     - Cierra la consola.                               |" << std::endl;
    std::cout << "+--------------------------------------------------------------------------------+" << std::endl;
}

void CLIHandlerNamespace::CLIHandler::processCommand(const std::string& full_command) {
    std::stringstream ss(full_command);
    std::string command;
    ss >> command;

    if (!currentUserInfo) {
        std::cout << "[CLI] Error crítico: No hay un usuario autenticado. Saliendo." << std::endl;
        return;
    }

    // Obtenemos las credenciales para cada llamada
    const auto& [username, password, role] = *currentUserInfo;
    bool isAdmin = (role == UserRole::ADMIN);

    try {
        xmlrpc_c::value result;
        if (command == "conectar") {
            if (!isAdmin) {
                std::cout << "[CLI] Error: Permiso denegado. Solo los administradores pueden conectar." << std::endl;
                return;
            }
            rpcClient.call(serverUrl, "robot.connect", "ss", &result, username.c_str(), password.c_str());
            std::cout << "[CLI] Comando 'conectar' enviado." << std::endl;
        } else if (command == "desconectar") {
            if (!isAdmin) {
                std::cout << "[CLI] Error: Permiso denegado. Solo los administradores pueden desconectar." << std::endl;
                return;
            }
            rpcClient.call(serverUrl, "robot.disconnect", "ss", &result, username.c_str(), password.c_str());
            std::cout << "[CLI] Comando 'desconectar' enviado." << std::endl;
        } else if (command == "motores_on") {
            rpcClient.call(serverUrl, "robot.enableMotors", "ss", &result, username.c_str(), password.c_str());
            std::cout << "[CLI] Comando 'motores_on' enviado." << std::endl;
        } else if (command == "motores_off") {
            rpcClient.call(serverUrl, "robot.disableMotors", "ss", &result, username.c_str(), password.c_str());
            std::cout << "[CLI] Comando 'motores_off' enviado." << std::endl;
        } else if (command == "efector_on") {
            rpcClient.call(serverUrl, "robot.setEffector", "ssb", &result, username.c_str(), password.c_str(), true);
            std::cout << "[CLI] Comando 'efector_on' enviado." << std::endl;
        } else if (command == "efector_off") {
            rpcClient.call(serverUrl, "robot.setEffector", "ssb", &result, username.c_str(), password.c_str(), false);
            std::cout << "[CLI] Comando 'efector_off' enviado." << std::endl;
        } else if (command == "mover") {
            double x, y, z, speed;
            if (ss >> x >> y >> z >> speed) {
                rpcClient.call(serverUrl, "robot.move", "ssdddd", &result, username.c_str(), password.c_str(), x, y, z, speed);
                std::cout << "[CLI] Comando 'mover' enviado." << std::endl;
            } else {
                std::cout << "[CLI] Error: Sintaxis incorrecta. Uso: mover <x> <y> <z> <velocidad>" << std::endl;
            }
        } else if (command == "user_add") {
            if (!isAdmin) {
                std::cout << "[CLI] Error: Permiso denegado. Solo los administradores pueden añadir usuarios." << std::endl;
                return;
            }
            
            std::string newUsername, newPassword;
            int roleInt;
            if (ss >> newUsername >> newPassword >> roleInt) {
                if (roleInt != 0 && roleInt != 1) {
                    std::cout << "[CLI] Error: Rol inválido. Use 0 para ADMIN o 1 para OPERATOR." << std::endl;
                    return;
                }
                // Llamamos al método RPC con las credenciales del admin y los datos del nuevo usuario.
                rpcClient.call(serverUrl, "robot.user_add", "ssssi", &result, username.c_str(), password.c_str(), newUsername.c_str(), newPassword.c_str(), roleInt);
                std::cout << "[CLI] Solicitud para crear usuario '" << newUsername << "' enviada." << std::endl;
            } else {
                std::cout << "[CLI] Error: Sintaxis incorrecta. Uso: user_add <user> <pass> <role>" << std::endl;
            }
        } else if (command == "estado") {
            rpcClient.call(serverUrl, "robot.getStatus", "ss", &result, username.c_str(), password.c_str());
            
            // Parseamos la estructura de la respuesta
            // 1. Convertimos el resultado RPC a un objeto 'value_struct'.
            xmlrpc_c::value_struct const statusStructValue(result);
            // 2. Convertimos el 'value_struct' a un mapa estándar de C++.
            std::map<std::string, xmlrpc_c::value> const statusMap(statusStructValue);

            bool isConnected = xmlrpc_c::value_boolean(statusMap.at("isConnected"));
            bool areMotorsEnabled = xmlrpc_c::value_boolean(statusMap.at("areMotorsEnabled"));
            std::string activityState = xmlrpc_c::value_string(statusMap.at("activityState"));
            
            std::map<std::string, xmlrpc_c::value> const posMap(xmlrpc_c::value_struct(statusMap.at("position")));
            double posX = xmlrpc_c::value_double(posMap.at("x"));
            double posY = xmlrpc_c::value_double(posMap.at("y"));
            double posZ = xmlrpc_c::value_double(posMap.at("z"));

            std::cout << "\n--- Estado del Robot (Remoto) ---" << std::endl;
            std::cout << "Conectado: " << (isConnected ? "Sí" : "No") << std::endl;
            std::cout << "Motores Activados: " << (areMotorsEnabled ? "Sí" : "No") << std::endl;
            std::cout << "Estado de Actividad: " << activityState << std::endl;
            std::cout << "Posición Actual: (" << posX << ", " << posY << ", " << posZ << ")" << std::endl;
            std::cout << "---------------------------------" << std::endl;
        } else {
            std::cout << "[CLI] Comando '" << command << "' no reconocido." << std::endl;
        }
    } catch (const girerr::error& e) {
        std::cerr << "[CLI] Error de conexión con el servidor: " << e.what() << std::endl;
        std::cerr << "La conexión puede haberse perdido. Intente reiniciar el cliente." << std::endl;
    } catch (const xmlrpc_c::fault& f) {
        std::cerr << "[CLI] Error RPC al ejecutar comando: " << f.getDescription() << std::endl;
    }
}
