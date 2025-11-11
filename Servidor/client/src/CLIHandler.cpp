#include "CLIHandler.h"
#include <iostream>
#include <string>
#include <limits> // Para std::numeric_limits
#include <sstream> // Para std::stringstream
#include <iomanip> // Para std::setw y std::left
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

std::string generateGCode(double x, double y, double z, double speed = 0.0);


// Methods

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
            std::string roleStr = (roleInt == 0) ? "Administrador" : "Operador";
            std::cout << "¡Inicio de sesión exitoso! Bienvenido, " << username << " (" << roleStr << ")." << std::endl;
            // Guardamos las credenciales y el rol para usarlas en futuras llamadas.
            currentUserInfo = std::make_tuple(username, password, roleInt);
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
        if (currentUserInfo && std::get<2>(*currentUserInfo) == 0) {
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
    std::cout << "\n+-----------------------------------------------------------------------------------------+" << std::endl;
    std::cout << "|                      PANEL DE CONTROL DEL ROBOT                                         |" << std::endl;
    std::cout << "+-----------------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Conexión:                                                                   |" << std::endl;
    std::cout << "|   conectar                      - Conecta con el robot.                                 |" << std::endl;
    std::cout << "|   desconectar                   - Desconecta el robot.                                  |" << std::endl;
    std::cout << "+-----------------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Usuario:                                                                    |" << std::endl;
    std::cout << "|   user_add <user> <pass> <role> - Añade un usuario (role: 0=ADMIN, 1=OPERATOR)          |" << std::endl;
    std::cout << "+-----------------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Control:                                                                    |" << std::endl;
    std::cout << "|   motores_on                    - Activa los motores.                                   |" << std::endl;
    std::cout << "|   motores_off                   - Desactiva los motores.                                |" << std::endl;
    std::cout << "|   mover <x> <y> <z> [vel]       - Mueve el efector a una posición. 'vel' es opcional.   |" << std::endl;
    std::cout << "|   efector_on                    - Activa el efector final.                              |" << std::endl;
    std::cout << "|   efector_off                   - Desactiva el efector final.                           |" << std::endl;
    std::cout << "|   modo_absoluto                 - Fija el modo de coordenadas a absoluto.               |" << std::endl;
    std::cout << "|   modo_relativo                 - Fija el modo de coordenadas a relativo.               |" << std::endl;
    std::cout << "+-----------------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Tareas:                                                                     |" << std::endl;
    std::cout << "|   lista_tareas                  - Muestra las tareas predefinidas.                      |" << std::endl;
    std::cout << "|   ejecutar_tarea <id_tarea>     - Ejecuta una tarea por su ID.                          |" << std::endl;
    std::cout << "|   aprender_inicio <id> <nombre> - Inicia el modo de aprendizaje de trayectoria.         |" << std::endl;
    std::cout << "|   aprender_fin                  - Finaliza el aprendizaje y guarda la tarea.            |" << std::endl;
    std::cout << "+-----------------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Información y Salida:                                                       |" << std::endl;
    std::cout << "|   estado                        - Muestra el estado actual del robot.                   |" << std::endl;
    std::cout << "|   reporte                       - Muestra un reporte de actividad.                      |" << std::endl;
    std::cout << "|   report_admin [filtro] [val]   - Muestra reporte admin con filtros usuario, resultado. |" << std::endl;
    std::cout << "|   report_log   [filtro] [val]   - Muestra reporte log con filtros usuario, nivel.       |" << std::endl;
    std::cout << "|   ayuda                         - Muestra esta ayuda.                                   |" << std::endl;
    std::cout << "|   salir                         - Cierra la consola.                                    |" << std::endl;
    std::cout << "+-----------------------------------------------------------------------------------------+" << std::endl;
}

void CLIHandlerNamespace::CLIHandler::displayMenuOperator() {
    std::cout << "\n+--------------------------------------------------------------------------------+" << std::endl;
    std::cout << "|                      PANEL DE CONTROL DEL ROBOT                                |" << std::endl;
    std::cout << "+--------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Control:                                                           |" << std::endl;
    std::cout << "|   motores_on                - Activa los motores.                              |" << std::endl;
    std::cout << "|   motores_off               - Desactiva los motores.                           |" << std::endl;
    std::cout << "|   mover <x> <y> <z> [vel]   - Mueve el efector. 'vel' es opcional.             |" << std::endl;
    std::cout << "|   efector_on                - Activa el efector final.                         |" << std::endl;
    std::cout << "|   efector_off               - Desactiva el efector final.                      |" << std::endl;
    std::cout << "|   modo_absoluto             - Fija el modo de coordenadas a absoluto.          |" << std::endl;
    std::cout << "|   modo_relativo             - Fija el modo de coordenadas a relativo.          |" << std::endl;
    std::cout << "+--------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Tareas:                                                            |" << std::endl;
    std::cout << "|   lista_tareas              - Muestra las tareas predefinidas.                 |" << std::endl;
    std::cout << "|   ejecutar_tarea <id_tarea> - Ejecuta una tarea por su ID.                     |" << std::endl;
    std::cout << "|   aprender_inicio <id> <nombre> - Inicia el modo de aprendizaje.               |" << std::endl;
    std::cout << "|   aprender_fin              - Finaliza y guarda la tarea aprendida.            |" << std::endl;
    std::cout << "+--------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Comandos de Información y Salida:                                              |" << std::endl;
    std::cout << "|   estado                    - Muestra el estado actual del robot.              |" << std::endl;
    std::cout << "|   reporte                   - Muestra un reporte de actividad.                 |" << std::endl;
    std::cout << "|   ayuda                     - Muestra esta ayuda.                              |" << std::endl;
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
    bool isAdmin = (role == 0);

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
            if (learningModeActive) {
                learnedGCodeCommands.push_back("M18");
                std::cout << "[APRENDIZAJE] Comando 'M18' grabado." << std::endl;
            }
            std::cout << "[CLI] Comando 'motores_off' enviado." << std::endl;
        } else if (command == "efector_on") {
            rpcClient.call(serverUrl, "robot.setEffector", "ssb", &result, username.c_str(), password.c_str(), true);
            if (learningModeActive) {
                learnedGCodeCommands.push_back("M3");
                std::cout << "[APRENDIZAJE] Comando 'M3' grabado." << std::endl;
            }
            std::cout << "[CLI] Comando 'efector_on' enviado." << std::endl;
        } else if (command == "efector_off") {
            rpcClient.call(serverUrl, "robot.setEffector", "ssb", &result, username.c_str(), password.c_str(), false);
            if (learningModeActive) {
                learnedGCodeCommands.push_back("M5");
                std::cout << "[APRENDIZAJE] Comando 'M5' grabado." << std::endl;
            }
            std::cout << "[CLI] Comando 'efector_off' enviado." << std::endl;
        } else if (command == "mover") {
            double x, y, z;
            std::string gcode;
            if (!(ss >> x >> y >> z)) {
                std::cout << "[CLI] Error: Sintaxis incorrecta. Uso: mover <x> <y> <z> [velocidad]" << std::endl;
                return;
            }

            double speed;
            if (ss >> speed) {
                gcode = generateGCode(x, y, z, speed);
                // Si hay un cuarto parámetro (velocidad), llamamos al método original.
                rpcClient.call(serverUrl, "robot.move", "ssdddd", &result, username.c_str(), password.c_str(), x, y, z, speed);
                std::cout << "[CLI] Comando 'mover' con velocidad específica enviado." << std::endl;
            } else {
                gcode = generateGCode(x, y, z);
                // Si no hay cuarto parámetro, llamamos al nuevo método sin velocidad.
                rpcClient.call(serverUrl, "robot.moveDefaultSpeed", "ssddd", &result, username.c_str(), password.c_str(), x, y, z);
                std::cout << "[CLI] Comando 'mover' con velocidad por defecto enviado." << std::endl;
            }

            if (learningModeActive){
                learnedGCodeCommands.push_back(gcode);
                std::cout << "[APRENDIZAJE] Comando '" << gcode << "' grabado." << std::endl;
            }

        } else if (command == "modo_absoluto") {
            rpcClient.call(serverUrl, "robot.setCoordinateMode", "ssb", &result, username.c_str(), password.c_str(), true);
            if (learningModeActive) {
                learnedGCodeCommands.push_back("G90");
                std::cout << "[APRENDIZAJE] Comando 'G90' grabado." << std::endl;
            }
            std::cout << "[CLI] Modo de coordenadas fijado a ABSOLUTO (G90)." << std::endl;
        } else if (command == "modo_relativo") {
            rpcClient.call(serverUrl, "robot.setCoordinateMode", "ssb", &result, username.c_str(), password.c_str(), false);
            if (learningModeActive) {
                learnedGCodeCommands.push_back("G91");
                std::cout << "[APRENDIZAJE] Comando 'G91' grabado." << std::endl;
            }
            std::cout << "[CLI] Modo de coordenadas fijado a RELATIVO (G91)." << std::endl;
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
        } else if (command == "estado") { // Los otros clientes deberian llamar a M114.
            rpcClient.call(serverUrl, "robot.getStatus", "ss", &result, username.c_str(), password.c_str());
            
            // Parseamos la estructura de la respuesta
            // 1. Convertimos el resultado RPC a un objeto 'value_struct'.
            xmlrpc_c::value_struct const statusStructValue(result);
            // 2. Convertimos el 'value_struct' a un mapa estándar de C++.
            std::map<std::string, xmlrpc_c::value> const statusMap(statusStructValue);

            bool isConnected = xmlrpc_c::value_boolean(statusMap.at("isConnected"));
            bool areMotorsEnabled = xmlrpc_c::value_boolean(statusMap.at("areMotorsEnabled"));
            std::string activityState = xmlrpc_c::value_string(statusMap.at("activityState"));
            std::string coordinateMode = xmlrpc_c::value_string(statusMap.at("coordinateMode"));
            
            std::map<std::string, xmlrpc_c::value> const posMap(xmlrpc_c::value_struct(statusMap.at("position")));
            double posX = xmlrpc_c::value_double(posMap.at("x"));
            double posY = xmlrpc_c::value_double(posMap.at("y"));
            double posZ = xmlrpc_c::value_double(posMap.at("z"));

            std::cout << "\n--- Estado del Robot (Remoto) ---" << std::endl;
            std::cout << "Conectado: " << (isConnected ? "Sí" : "No") << std::endl;
            std::cout << "Motores Activados: " << (areMotorsEnabled ? "Sí" : "No") << std::endl;
            std::cout << "Modo de Coordenadas: " << coordinateMode << std::endl;
            std::cout << "Estado de Actividad: " << activityState << std::endl;
            std::cout << "Posición Actual: (" << std::fixed << std::setprecision(2) << posX << ", " << posY << ", " << posZ << ")" << std::endl;
            std::cout << "---------------------------------" << std::endl;

        } else if (command == "ayuda") {
            rpcClient.call(serverUrl, "robot.help", "ss", &result, username.c_str(), password.c_str());
            
            xmlrpc_c::value_struct const helpStructValue(result);
            std::map<std::string, xmlrpc_c::value> const helpMap(helpStructValue);

            std::cout << "\n--- Comandos Disponibles ---" << std::endl;
            for (const auto& pair : helpMap) {
                std::string const cmd = pair.first;
                std::string const helpText = xmlrpc_c::value_string(pair.second);
                // Usamos std::left y std::setw para alinear las columnas
                std::cout << "  " << std::left << std::setw(30) << cmd << " - " << helpText << std::endl;
            }
            std::cout << "----------------------------" << std::endl;
        } else if (command == "reporte") {
            rpcClient.call(serverUrl, "robot.getReport", "ss", &result, username.c_str(), password.c_str());

            std::map<std::string, xmlrpc_c::value> const reportMap{xmlrpc_c::value_struct(result)};

            // Imprimir información general
            std::cout << "\n--- Reporte de Actividad del Operador ---" << std::endl;
            std::cout << "Conectado: " << (xmlrpc_c::value_boolean(reportMap.at("isConnected")) ? "Sí" : "No") << std::endl;
            std::cout << "Motores: " << (xmlrpc_c::value_boolean(reportMap.at("areMotorsEnabled")) ? "Activados" : "Desactivados") << std::endl;
            std::cout << "Estado Actividad: " << xmlrpc_c::value_string(reportMap.at("activityState")).cvalue() << std::endl;
            
            std::map<std::string, xmlrpc_c::value> const posMap(xmlrpc_c::value_struct(reportMap.at("position")));
            std::cout << "Posición: (" << xmlrpc_c::value_double(posMap.at("x")) << ", " 
                                     << xmlrpc_c::value_double(posMap.at("y")) << ", " 
                                     << xmlrpc_c::value_double(posMap.at("z")) << ")" << std::endl;

            // Imprimir listado de órdenes
            std::cout << "\n--- Órdenes desde la última conexión ---" << std::endl;
            xmlrpc_c::value_array const ordersArray(reportMap.at("orders"));
            std::vector<xmlrpc_c::value> const ordersVector(ordersArray.vectorValueValue());

            if (ordersVector.empty()) {
                std::cout << "No se han registrado órdenes en esta sesión." << std::endl;
            } else {
                std::cout << std::left << std::setw(10) << "Hora"
                          << std::setw(15) << "Comando"
                          << std::setw(50) << "Detalles"
                          << std::setw(10) << "Resultado" << std::endl;
                std::cout << std::string(100, '-') << std::endl;
                for (const auto& orderValue : ordersVector) {
                    std::map<std::string, xmlrpc_c::value> const orderMap{xmlrpc_c::value_struct(orderValue)};
                    std::cout << std::left << std::setw(10) << xmlrpc_c::value_string(orderMap.at("timestamp")).cvalue()
                              << std::setw(15) << xmlrpc_c::value_string(orderMap.at("command")).cvalue()
                              << std::setw(50) << xmlrpc_c::value_string(orderMap.at("details")).cvalue()
                              << std::setw(15) << (xmlrpc_c::value_string(orderMap.at("success")).cvalue()) << std::endl;
                }
            }
            std::cout << std::string(100, '-') << std::endl;
        } else if (command == "report_admin") {
            if (!isAdmin) {
                std::cout << "[CLI] Error: Permiso denegado. Solo los administradores pueden usar 'reporte_admin'." << std::endl;
                return;
            }
            std::string filterKey, filterValue;
            ss >> filterKey >> filterValue; // Intenta leer la clave y el valor del filtro

            // Validamos que el filtro sea uno de los permitidos
            if (!filterKey.empty() && filterKey != "usuario" && filterKey != "resultado") {
                std::cout << "[CLI] Error: Filtro '" << filterKey << "' no válido. Use 'usuario' o 'resultado'." << std::endl;
                return;
            }
            if (filterKey == "usuario") filterKey = "username"; // Mapeamos al nombre interno

            rpcClient.call(serverUrl, "robot.getAdminReport", "ssss", &result, username.c_str(), password.c_str(), filterKey.c_str(), filterValue.c_str());
            
            std::map<std::string, xmlrpc_c::value> const reportMap{xmlrpc_c::value_struct(result)};
            xmlrpc_c::value_array const ordersArray(reportMap.at("orders"));
            std::vector<xmlrpc_c::value> const ordersVector(ordersArray.vectorValueValue());

            std::cout << "\n--- Reporte de Administrador ---" << std::endl;
            if (!filterKey.empty()) {
                std::cout << "Filtro aplicado: " << filterKey << " = '" << filterValue << "'" << std::endl;
            }

            if (ordersVector.empty()) {
                std::cout << "No se encontraron órdenes que coincidan con el filtro." << std::endl;
            } else {
                std::cout << std::left << std::setw(10) << "Hora"
                          << std::setw(10) << "Usuario"
                          << std::setw(17) << "Comando"
                          << std::setw(46) << "Detalles"
                          << std::setw(20) << "Resultado" << std::endl;
                std::cout << std::string(105, '-') << std::endl;
                for (const auto& orderValue : ordersVector) {
                    std::map<std::string, xmlrpc_c::value> const orderMap{xmlrpc_c::value_struct(orderValue)};
                    std::cout << std::left 
                              << std::setw(10) << xmlrpc_c::value_string(orderMap.at("timestamp")).cvalue()
                              << std::setw(10) << xmlrpc_c::value_string(orderMap.at("username")).cvalue()
                              << std::setw(17) << xmlrpc_c::value_string(orderMap.at("command")).cvalue()
                              << std::setw(46) << xmlrpc_c::value_string(orderMap.at("details")).cvalue()
                              << std::setw(20) << xmlrpc_c::value_string(orderMap.at("success")).cvalue() << std::endl;
                }
            }
            std::cout << std::string(105, '-') << std::endl;

        }else if (command == "report_log"){
            if (!isAdmin) {
                std::cout << "[CLI] Error: Permiso denegado. Solo los administradores pueden usar 'report_log'." << std::endl;
                return;
            }
            std::string filterKey, filterValue;
            ss >> filterKey >> filterValue; // Intenta leer la clave y el valor del filtro

            // Validamos que el filtro sea uno de los permitidos
            if (!filterKey.empty() && filterKey != "usuario" && filterKey != "nivel") {
                std::cout << "[CLI] Error: Filtro '" << filterKey << "' no válido. Use 'usuario' o 'nivel'." << std::endl;
                return;
            }
            if (filterKey == "usuario") filterKey = "username"; // Mapeamos al nombre interno
            if (filterKey == "nivel") filterKey = "level"; // Mapeamos al nombre interno

            rpcClient.call(serverUrl, "robot.getLogReport", "ssss", &result, username.c_str(), password.c_str(), filterKey.c_str(), filterValue.c_str());

            std::map<std::string, xmlrpc_c::value> const reportMap{xmlrpc_c::value_struct(result)};
            xmlrpc_c::value_array const logsArray(reportMap.at("logs"));
            std::vector<xmlrpc_c::value> const logsVector(logsArray.vectorValueValue());

            std::cout << "\n--- Reporte de Logs del Servidor ---" << std::endl;
            if (!filterKey.empty()) {
                std::cout << "Filtro aplicado: " << filterKey << " = '" << filterValue << "'" << std::endl;
            }

            if (logsVector.empty()) {
                std::cout << "No se encontraron logs que coincidan con el filtro." << std::endl;
            } else {
                std::cout << std::left << std::setw(22) << "Fecha y Hora"
                          << std::setw(10) << "Nivel"
                          << std::setw(15) << "Usuario"
                          << std::setw(40) << "Mensaje"
                          << std::setw(20) << "Nodo" << std::endl;
                std::cout << std::string(107, '-') << std::endl;
                for (const auto& logValue : logsVector) {
                    std::map<std::string, xmlrpc_c::value> const logMap{xmlrpc_c::value_struct(logValue)};
                    std::cout << std::left 
                              << std::setw(22) << xmlrpc_c::value_string(logMap.at("timestamp")).cvalue()
                              << std::setw(10) << xmlrpc_c::value_string(logMap.at("level")).cvalue()
                              << std::setw(15) << xmlrpc_c::value_string(logMap.at("user")).cvalue()
                              << std::setw(40) << xmlrpc_c::value_string(logMap.at("message")).cvalue()
                              << std::setw(20) << xmlrpc_c::value_string(logMap.at("node")).cvalue() << std::endl;
                }
            }
            std::cout << std::string(107, '-') << std::endl;
        } else if (command == "lista_tareas") {
            rpcClient.call(serverUrl, "robot.listTasks", "ss", &result, username.c_str(), password.c_str());

            xmlrpc_c::value_array const tasksArray(result);
            std::vector<xmlrpc_c::value> const tasksVector(tasksArray.vectorValueValue());

            std::cout << "\n--- Tareas Predefinidas Disponibles ---" << std::endl;
            if (tasksVector.empty()) {
                std::cout << "No hay tareas disponibles." << std::endl;
            } else {
                std::cout << std::left << std::setw(20) << "ID de Tarea"
                          << std::setw(30) << "Nombre"
                          << "Descripción" << std::endl;
                std::cout << std::string(100, '-') << std::endl;
                for (const auto& taskValue : tasksVector) {
                    std::map<std::string, xmlrpc_c::value> const taskMap{xmlrpc_c::value_struct(taskValue)};
                    std::cout << std::left << std::setw(20) << xmlrpc_c::value_string(taskMap.at("id")).cvalue()
                              << std::setw(30) << xmlrpc_c::value_string(taskMap.at("name")).cvalue()
                              << xmlrpc_c::value_string(taskMap.at("description")).cvalue() << std::endl;
                }
            }
            std::cout << std::string(100, '-') << std::endl;
        } else if (command == "ejecutar_tarea") {
            std::string taskId;
            if (ss >> taskId) {
                rpcClient.call(serverUrl, "robot.executeTask", "sss", &result, username.c_str(), password.c_str(), taskId.c_str());
                std::cout << "[CLI] Solicitud para ejecutar la tarea '" << taskId << "' enviada." << std::endl;
            } else {
                std::cout << "[CLI] Error: Sintaxis incorrecta. Uso: ejecutar_tarea <id_tarea>" << std::endl;
            }
        } else if (command == "aprender_inicio") {
            if (learningModeActive) {
                std::cout << "[CLI] Error: Ya estás en modo de aprendizaje. Usa 'aprender_fin' para terminar." << std::endl;
                return;
            }
            std::string id, name;
            if (ss >> id >> name) {
                learningModeActive = true;
                learningTaskId = id;
                learningTaskName = name;
                learnedGCodeCommands.clear();
                std::cout << "[APRENDIZAJE] Modo de aprendizaje iniciado para la tarea '" << id << "'." << std::endl;
                std::cout << "[APRENDIZAJE] Todos los comandos de movimiento se grabarán. Usa 'aprender_fin' para guardar." << std::endl;
            } else {
                std::cout << "[CLI] Error: Sintaxis incorrecta. Uso: aprender_inicio <id_tarea> <nombre_tarea>" << std::endl;
            }
        } else if (command == "aprender_fin") {
            if (!learningModeActive) {
                std::cout << "[CLI] Error: No estás en modo de aprendizaje. Usa 'aprender_inicio' para empezar." << std::endl;
                return;
            }

            learningModeActive = false;
            if (learnedGCodeCommands.empty()) {
                std::cout << "[APRENDIZAJE] No se grabaron comandos. Tarea no guardada." << std::endl;
            } else {

                // Crear el vector de comandos
                std::vector<xmlrpc_c::value> paramArray;
                for (const auto& cmd : learnedGCodeCommands) {
                    paramArray.push_back(xmlrpc_c::value_string(cmd));
                }
                
                // Crear el array
                xmlrpc_c::value_array gcodeArray(paramArray);

                // Crear un paramList y añadir todos los argumentos
                xmlrpc_c::paramList params;
                params.add(xmlrpc_c::value_string(username));
                params.add(xmlrpc_c::value_string(password));
                params.add(xmlrpc_c::value_string(learningTaskId));
                params.add(xmlrpc_c::value_string(learningTaskName));
                params.add(gcodeArray); // Añadir el xmlrpc_c::value_array directamente

                // Llamar al método RPC usando la sobrecarga con paramList
                rpcClient.call(serverUrl, "robot.addTask", params, &result);
                std::cout << "[APRENDIZAJE] Tarea '" << learningTaskId << "' guardada en el servidor con " << learnedGCodeCommands.size() << " comandos." << std::endl;
            }
        } else {
            std::cout << "[CLI] Comando '" << command << "' no reconocido." << std::endl;
        }
    } catch (const girerr::error& e) {
        std::cerr << "[CLI] Error de conexión con el servidor: " << e.what() << std::endl;
    } catch (const xmlrpc_c::fault& f) {
        std::cerr << "[CLI] Error RPC al ejecutar comando: " << f.getDescription() << std::endl;
    }
}

std::string generateGCode(double x, double y, double z, double speed) {
    std::stringstream ss;
    // G1 es el comando para movimiento lineal. F es para la velocidad (Feed rate).
    ss << "G1 "
       << "X" << std::fixed << std::setprecision(3) << x << " "
       << "Y" << std::fixed << std::setprecision(3) << y << " "
       << "Z" << std::fixed << std::setprecision(3) << z << " "
       << "E" << std::fixed << std::setprecision(1) << speed;
    return ss.str();
}