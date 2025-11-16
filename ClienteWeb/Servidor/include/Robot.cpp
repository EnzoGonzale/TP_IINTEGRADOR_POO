#include "Robot.h"
#include <iostream>
#include "GCode.h" // Incluimos la clase GCode para usar su funcionalidad
#include <stdexcept>  // Para std::runtime_error
#include <thread>     // Para std::this_thread::sleep_for
#include <unistd.h> // Para usleep
#include <algorithm>  // Para std::remove
#include <iomanip>    // Para std::put_time
#include <sstream>
#include <regex>      // Para expresiones regulares

// --- Declaración de la nueva función privada ---
static std::string sendAndReceive(ComunicatorPort::SerialComunicator& serial, const std::string& command, int time = 2);


RobotNamespace::Robot::Robot() : isConnected(false),
                 areMotorsEnabled(false),
                 activityState("IDLE"),
                 isAbsolute_(true), // Inicializamos el nuevo miembro
                 serialCommunicator() // Llama al constructor por defecto de SerialComunicator
{
}

RobotNamespace::Robot::~Robot()
{
}

/// @brief Parsea la respuesta del comando M114 y actualiza el estado del robot.
void RobotNamespace::Robot::parseM114Response(const std::string& response) {
    std::stringstream ss(response);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.find("ABSOLUTE MODE") != std::string::npos) {
            isAbsolute_ = true;
        } else if (line.find("RELATIVE MODE") != std::string::npos) {
            isAbsolute_ = false;
        } else if (line.find("MOTORS ENABLED") != std::string::npos) {
            areMotorsEnabled = true;
        } else if (line.find("MOTORS DISABLED") != std::string::npos) {
            areMotorsEnabled = false;
        } else if (line.find("CURRENT POSITION:") != std::string::npos) {
            // Expresión regular para extraer los valores de X, Y, Z
            std::regex re(R"(X:([-\d.]+) Y:([-\d.]+) Z:([-\d.]+))");
            std::smatch match;
            if (std::regex_search(line, match, re) && match.size() == 4) {
                try {
                    currentPosition.x = std::stod(match[1].str());
                    currentPosition.y = std::stod(match[2].str());
                    currentPosition.z = std::stod(match[3].str());
                } catch (const std::invalid_argument& e) {
                    std::cerr << "[Robot] Error al convertir posición desde M114: " << e.what() << std::endl;
                }
            }
        }
    }
}

RobotStatus RobotNamespace::Robot::getStatus() {
    if (isConnected) {
        std::string response = sendAndReceive(serialCommunicator, "M114\r\n");
        parseM114Response(response);
    }

    RobotStatus status;
    status.isConnected = isConnected;
    status.activityState = activityState;
    status.areMotorsEnabled = areMotorsEnabled;
    status.currentPosition = currentPosition;
    status.isAbsolute = isAbsolute_;
    return status;
}

void RobotNamespace::Robot::connect() {
    if (!isConnected) {
        std::cout << "[Robot] Conectando..." << std::endl;
        serialCommunicator.config("/dev/ttyUSB0", 115200);

        // Al abrir el puerto, el Arduino se reinicia. Esperamos un tiempo prudencial
        // para que termine su secuencia de arranque y envíe cualquier mensaje inicial.
        std::cout << "[Robot] Esperando reinicio del dispositivo (2s)..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));

        serialCommunicator.cleanBuffer(); // Ahora limpiamos cualquier mensaje de arranque.
        isConnected = true;
        lastOrders.clear(); // Limpiamos el historial de órdenes al conectar.
        activityState = "CONECTADO";
        coutExecuteState("[Robot] Conexión establecida.");
    } else {
        coutExecuteState("[Robot] Advertencia: El robot ya está conectado.");
    }
}

void RobotNamespace::Robot::disconnect() {
    if (isConnected) {
        std::cout << "[Robot] Desconectando..." << std::endl;
        serialCommunicator.close();
        isConnected = false;
        areMotorsEnabled = false; // Al desconectar, los motores se apagan
        activityState = "DESCONECTADO";
        coutExecuteState("[Robot] Desconexión completada.");
    } else {
        coutExecuteState("[Robot] Advertencia: El robot ya está desconectado.");
    }
}

void RobotNamespace::Robot::enableMotors() {
    if (activityState == "MOVIENDO") {
        coutExecuteState("[Robot] Error: No se pueden modificar los motores mientras el robot está en movimiento.");
        return;
    }
    if (isConnected && !areMotorsEnabled) {
        areMotorsEnabled = true;
        std::string response = sendAndReceive(serialCommunicator, "M17\r\n");
        std::cout << "[Robot] Respuesta de M17: " << (response.empty() ? "[ninguna]" : response) << std::endl;
        coutExecuteState("[Robot] Motores activados.");
    } else if (!isConnected) {
        coutExecuteState("[Robot] Error: No se pueden activar los motores. El robot no está conectado.");
    } else {
        coutExecuteState("[Robot] Advertencia: Los motores ya están activados.");
    }
}

void RobotNamespace::Robot::disableMotors() {
    if (activityState == "MOVIENDO") {
        coutExecuteState("[Robot] Error: No se pueden modificar los motores mientras el robot está en movimiento.");
        return;
    }
    if (areMotorsEnabled) {
        areMotorsEnabled = false;
        std::string response = sendAndReceive(serialCommunicator, "M18\r\n");
        coutExecuteState("[Robot] Motores desactivados.");
        std::cout << "[Robot] Respuesta de M18: " << (response.empty() ? "[ninguna]" : response) << std::endl;
    }
}

void RobotNamespace::Robot::moveTo(const Position& position, double speed) {
    executeMivement(const_cast<Position&>(position), speed);
}

void RobotNamespace::Robot::moveTo(const Position& position) {
    executeMivement(const_cast<Position&>(position));
}


void RobotNamespace::Robot::sendRawGCode(const std::string& gcode) {
    if (activityState == "MOVIENDO") {
        coutExecuteState("[Robot] Error: No se puede enviar G-Code mientras el robot está en movimiento.");
        return;
    }
    if (isConnected) {
        std::cout << "[Robot] Enviando G-Code crudo: \"" << gcode << "\"" << std::endl;
        std::string response = sendAndReceive(serialCommunicator, gcode + "\r\n");
        coutExecuteState("[Robot] Respuesta a G-Code crudo: " + (response.empty() ? "[ninguna]" : response));
    } else {
        coutExecuteState("[Robot] Error: No se puede enviar G-Code. El robot no está conectado.");
    }
}


void RobotNamespace::Robot::setEffector(bool active) {
    if (activityState == "MOVIENDO") {
        coutExecuteState("[Robot] Error: No se puede modificar el efector final mientras el robot está en movimiento.");
        return;
    }
    if (isConnected) {
        if (active) {
            std::string response = sendAndReceive(serialCommunicator, "M3\r\n");
            std::cout << "[Robot] Respuesta de M3: " << (response.empty() ? "[ninguna]" : response) << std::endl;
        } else {
            std::string response = sendAndReceive(serialCommunicator, "M5\r\n");
            std::cout << "[Robot] Respuesta de M5: " << (response.empty() ? "[ninguna]" : response) << std::endl;
        }
        std::string efector = active ? "activado" : "desactivado";
        coutExecuteState("[Robot] Efector final " + efector + ".");
    } else {
        coutExecuteState("[Robot] Error: No se puede modificar el efector final. El robot no está conectado.");
    }
}

void RobotNamespace::Robot::setCoordinateMode(bool isAbsolute) {
    if (isConnected) {
        std::string command = isAbsolute ? "G90\r\n" : "G91\r\n";
        isAbsolute_ = isAbsolute; // Actualizamos el estado interno inmediatamente
        std::string response = sendAndReceive(serialCommunicator, command);
        response = response.empty() ? "[ninguna]" : response;
        
        std::ostringstream message;
        message << "[Robot] Respuesta de " 
                << (isAbsolute ? "G90" : "G91") 
                << ": " << response;
        coutExecuteState(message.str());
    } else {
        coutExecuteState("[Robot] Error: No se puede cambiar el modo de coordenadas. El robot no está conectado.");
    }
}


void RobotNamespace::Robot::recordOrder(const std::string& username, const std::string& commandName, 
                                         const std::string& details) {
    Order newOrder;
    
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");

    newOrder.timestamp = ss.str();
    newOrder.username = username;
    newOrder.commandName = commandName;
    newOrder.details = details;
    newOrder.success = getExecuteState();

    lastOrders.push_back(newOrder);
}

void RobotNamespace::Robot::coutExecuteState(std::string state) {
    setExecuteState(state);
    std::cout << getExecuteState() << std::endl;
}

void RobotNamespace::Robot::initAttributes()
{
}

/// @brief Envía un comando y espera activamente una respuesta.
static std::string sendAndReceive(ComunicatorPort::SerialComunicator& serial, const std::string& command, int time) {
    serial.sendMessage(command);
    
    std::string full_response;
    // Intentamos leer varias veces, ya que la respuesta puede llegar en fragmentos.
    full_response = serial.reciveMessage(time);

    // 1. Reemplazamos la respuesta "OK" para que sea más legible.
    // Usamos un bucle por si aparece varias veces.
    size_t pos = 0;
    std::string replacement = ". OK. ";
    while ((pos = full_response.find("OK", pos)) != std::string::npos) {
        full_response.replace(pos, 2, replacement);
        pos += replacement.length(); // Avanzamos el cursor después de la cadena insertada.
    }

    // 2. Eliminamos todos los caracteres de nueva línea y retorno de carro restantes.
    full_response.erase(std::remove(full_response.begin(), full_response.end(), '\n'), full_response.end());
    full_response.erase(std::remove(full_response.begin(), full_response.end(), '\r'), full_response.end());

    return full_response; // Devolvemos la cadena procesada.
}

void RobotNamespace::Robot::executeMivement(Position& position, double speed){
    if (activityState == "MOVIENDO") {
        coutExecuteState("[Robot] Error: No se puede mover mientras el robot está en movimiento.");
        return;
    }
    if (isConnected && areMotorsEnabled) {
        // 1. Generar el comando G-Code a partir de la posición y velocidad.
        std::string gcodeCommand;
        if(position.x == 0 && position.y == 0 && position.z == 0) {
            coutExecuteState("[Robot] Moviendo al origen (0,0,0).");
            activityState = "ORIGEN";
            gcodeCommand = "G24\r\n";
        } else {
            if (speed != 2000.0){
                gcodeCommand = GCodeNamespace::GCode::generateMoveCommand(position.x, position.y, position.z, speed);
                std::cout << "[Robot] Generado G-Code: \"" << gcodeCommand << "\"" << std::endl;
            } else {
                gcodeCommand = GCodeNamespace::GCode::generateMoveCommand(position.x, position.y, position.z);
                std::cout << "[Robot] Generado G-Code (velocidad por defecto): \"" << gcodeCommand << "\"" << std::endl;
            }
        }
        try {
            // 2. Enviar el comando a través del comunicador serie.
            std::cout << "[Robot] Enviando comando al puerto serie..." << std::endl;
            currentPosition = position;
            activityState = "MOVIENDO"; // Cambiar estado ANTES de enviar
            std::string response = sendAndReceive(serialCommunicator, gcodeCommand + "\r\n", 3); // Mayor timeout para movimientos
            std::cout << "[Robot] Comando de movimiento aceptado por el robot. Respuesta: " << response << std::endl;
            // NOTA: El robot ahora está físicamente en movimiento.
            // El estado se quedará en "MOVIENDO". Se necesita un mecanismo (hilo, sondeo) para detectar el fin del movimiento.
            // Por ahora, lo cambiaremos a EN_POSICION para que no se bloquee.
            activityState = "EN_POSICION";
            coutExecuteState("[Robot] Movimiento completado.");
        } catch (const std::runtime_error& e) {
            std::cerr << "[Robot] Error al enviar comando de movimiento: " << e.what() << std::endl;
            coutExecuteState("[Robot] Error al enviar comando de movimiento.");
            activityState = "ERROR";
            return; // Salimos si hubo un error al enviar.
        }

        // Para ver el funcionamiento de la comunicacion serie
        // La respuesta ya se gestionó en sendCommandAndWaitForResponse

    } else {
        coutExecuteState("[Robot] Error: No se puede mover. Verifique la conexión y el estado de los motores.");
    }
}
