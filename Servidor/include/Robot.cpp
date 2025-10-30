#include "Robot.h"
#include <thread>           // Para std::this_thread::sleep_for
#include <unistd.h>         // Para usleep
#include <algorithm>        // Para std::remove
#include <iomanip>          // Para std::put_time
#include <sstream>
#include <regex>            // Para expresiones regulares
#include "ServiceLocator.h" // Incluimos el Service Locator
#include "GCode.h"          // Incluimos la clase GCode para usar su funcionalidad
#include "Exceptions.h"
#include "Utils.h"

// --- Declaración de la nueva función privada ---
static std::string sendAndReceive(ComunicatorPort::ISerialCommunicator& serial, const std::string& command, int time = 2);


RobotNamespace::Robot::Robot()
{
}

RobotNamespace::Robot::~Robot()
{
}

/// @brief Parsea la respuesta del comando M114 y actualiza el estado del robot.
void RobotNamespace::Robot::parseM114Response(const std::string& response) {
    // La respuesta puede no tener saltos de línea, así que buscamos en toda la cadena.
    if (response.find("ABSOLUTE MODE") != std::string::npos) {
        robotStatus.isAbsolute = true;
    } else if (response.find("RELATIVE MODE") != std::string::npos) {
        robotStatus.isAbsolute = false;
    }
    if (response.find("MOTORS ENABLED") != std::string::npos) {
        robotStatus.areMotorsEnabled = true;
    } else if (response.find("MOTORS DISABLED") != std::string::npos) {
        robotStatus.areMotorsEnabled = false;
    }

    // Expresión regular para extraer los valores de X, Y, Z
    std::regex re(R"(X:([-\d.]+) Y:([-\d.]+) Z:([-\d.]+))");
    std::smatch match;
    if (std::regex_search(response, match, re) && match.size() == 4) {
        try {
            robotStatus.currentPosition.x = std::stod(match[1].str());
            robotStatus.currentPosition.y = std::stod(match[2].str());
            robotStatus.currentPosition.z = std::stod(match[3].str());
        } catch (const std::invalid_argument& e) {
            exceptionAndExecute("[Robot] Error al convertir posición desde M114: " + std::string(e.what()));
        }
    }
}

RobotStatus RobotNamespace::Robot::getStatus() {
    std::string response;
    if (robotStatus.isConnected) {
        response = sendAndReceive(ServiceLocator::getCommunicator(), "M114\r\n");
        parseM114Response(response);
    }
    return robotStatus;
}

void RobotNamespace::Robot::connect() {
    if (!robotStatus.isConnected) {
        try{
            Logger::getInstance().log(LogLevel::INFO, "[Robot] Iniciando conexión...");
            ServiceLocator::getCommunicator().config("/dev/ttyUSB0", 115200);

            // Al abrir el puerto, el Arduino se reinicia. Esperamos un tiempo prudencial
            // para que termine su secuencia de arranque y envíe cualquier mensaje inicial.

            Logger::getInstance().log(LogLevel::INFO, "[Robot] Limpiando mensajes de arranque...");
            std::this_thread::sleep_for(std::chrono::seconds(2));

            ServiceLocator::getCommunicator().cleanBuffer(); // Ahora limpiamos cualquier mensaje de arranque.
            robotStatus.isConnected = true;
            lastOrders.clear(); // Limpiamos el historial de órdenes al conectar.
            robotStatus.activityState = "CONECTADO";
            logAndExecuteState(LogLevel::INFO, "[Robot] Conexión establecida.");
        } catch (const SerialCommunicationException& e){
            logAndExecuteState(LogLevel::CRITICAL, "[Robot] Fallo crítico al conectar: " + std::string(e.what()));
            throw; // Relanzamos la excepción para que RpcServiceHandler la capture.
        }
    } else {
        exceptionAndExecute("El robot ya está conectado.");
    }
}

void RobotNamespace::Robot::disconnect() {
    if (robotStatus.isConnected) {
        try{
            Logger::getInstance().log(LogLevel::INFO, "[Robot] Cerrando conexión...");
            ServiceLocator::getCommunicator().close();
            robotStatus.isConnected = false;
            robotStatus.areMotorsEnabled = false; // Al desconectar, los motores se apagan
            robotStatus.activityState = "DESCONECTADO";
            logAndExecuteState(LogLevel::INFO, "[Robot] Desconexión completada.");
        } catch (const SerialCommunicationException& e){
            logAndExecuteState(LogLevel::ERROR, "[Robot] Error al desconectar: " + std::string(e.what()));
            throw; // Relanzamos la excepción para que RpcServiceHandler la capture.
        }
    } else {
        exceptionAndExecute("El robot ya está desconectado.");
    }
}

void RobotNamespace::Robot::enableMotors() {
    isMoving();
    if (robotStatus.isConnected && !robotStatus.areMotorsEnabled) {
        try {
            std::string response = sendAndReceive(ServiceLocator::getCommunicator(), "M17\r\n");
            Logger::getInstance().log(LogLevel::INFO, "[Robot] Respuesta de M17: " + (response.empty() ? "[ninguna]" : response));
            logAndExecuteState(LogLevel::INFO, "[Robot] Motores activados.");
            robotStatus.areMotorsEnabled = true;
        } catch (const SerialCommunicationException& e) {
            robotStatus.areMotorsEnabled = false; // Revertimos el estado si falla la comunicación
            logAndExecuteState(LogLevel::ERROR, "[Robot] Error al activar motores: " + std::string(e.what()));
            throw;
        }
    } else if (!robotStatus.isConnected) {
        exceptionAndExecute("No se pueden activar los motores. El robot no está conectado.");
    } else {
        exceptionAndExecute("Los motores ya están activados.");
    }
}

void RobotNamespace::Robot::disableMotors() {
    isMoving();
    if (robotStatus.areMotorsEnabled) {
        try {
            std::string response = sendAndReceive(ServiceLocator::getCommunicator(), "M18\r\n");
            Logger::getInstance().log(LogLevel::INFO, "[Robot] Respuesta de M18: " + (response.empty() ? "[ninguna]" : response));
            logAndExecuteState(LogLevel::INFO, "[Robot] Motores desactivados.");
            robotStatus.areMotorsEnabled = false;
        } catch (const SerialCommunicationException& e) {
            robotStatus.areMotorsEnabled = true; // Revertimos el estado
            logAndExecuteState(LogLevel::ERROR, "[Robot] Error al desactivar motores: " + std::string(e.what()));
            throw;
        }
    } else {
        // Si los motores ya están desactivados, notificamos con una excepción.
        exceptionAndExecute("Los motores ya están desactivados.");
    }
}

void RobotNamespace::Robot::moveTo(const Position& position, double speed) {
    executeMoviment(const_cast<Position&>(position), speed);
}

void RobotNamespace::Robot::moveTo(const Position& position) {
    executeMoviment(const_cast<Position&>(position));
}

void RobotNamespace::Robot::sendRawGCode(const std::string& gcode) {
    isMoving();
    if (robotStatus.isConnected) {
        try {
            Logger::getInstance().log(LogLevel::INFO, "[Robot] Enviando G-Code crudo: \"" + gcode + "\"");
            std::string response = sendAndReceive(ServiceLocator::getCommunicator(), gcode + "\r\n");
            logAndExecuteState(LogLevel::INFO, "[Robot] Respuesta a G-Code crudo: " + (response.empty() ? "[ninguna]" : response));
        } catch (const SerialCommunicationException& e) {
            logAndExecuteState(LogLevel::ERROR, "[Robot] Error al enviar G-Code: " + std::string(e.what()));
            throw;
        }
    } else {
        exceptionAndExecute("No se puede enviar G-Code. El robot no está conectado.");
    }
}


void RobotNamespace::Robot::setEffector(bool active) {
    isMoving();
    if (robotStatus.isConnected) {
        try {
            if (active) {
                std::string response = sendAndReceive(ServiceLocator::getCommunicator(), "M3\r\n");
                Logger::getInstance().log(LogLevel::INFO, "[Robot] Respuesta de M3: " + (response.empty() ? "[ninguna]" : response));
            } else {
                std::string response = sendAndReceive(ServiceLocator::getCommunicator(), "M5\r\n");
                Logger::getInstance().log(LogLevel::INFO, "[Robot] Respuesta de M5: " + (response.empty() ? "[ninguna]" : response));
            }
            std::string efector = active ? "activado" : "desactivado";
            logAndExecuteState(LogLevel::INFO, "[Robot] Efector final " + efector + ".");
        } catch (const SerialCommunicationException& e) {
            logAndExecuteState(LogLevel::ERROR, "[Robot] Error al cambiar estado del efector: " + std::string(e.what()));
            throw;
        }
    } else {
        exceptionAndExecute("No se puede modificar el efector final. El robot no está conectado.");
    }
}

void RobotNamespace::Robot::setCoordinateMode(bool isAbsolute) {
    if (robotStatus.isConnected) {
        try{
            std::string command = isAbsolute ? "G90\r\n" : "G91\r\n";
            robotStatus.isAbsolute = isAbsolute; // Actualizamos el estado interno inmediatamente
            std::string response = sendAndReceive(ServiceLocator::getCommunicator(), command);
            response = response.empty() ? "[ninguna]" : response;
            
            std::ostringstream message;
            message << "[Robot] Respuesta de " 
                    << (isAbsolute ? "G90" : "G91") 
                    << ": " << response;
            logAndExecuteState(LogLevel::INFO, message.str());
        } catch (const SerialCommunicationException& e){
            logAndExecuteState(LogLevel::ERROR, "[Robot] Error al cambiar modo de coordenadas: " + std::string(e.what()));
            throw;
        }
    } else {
        exceptionAndExecute("No se puede cambiar el modo de coordenadas. El robot no está conectado.");
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

void RobotNamespace::Robot::logAndExecuteState(LogLevel level, std::string state) {
    setExecuteState(state);
    if (level == LogLevel::INFO) {
        Logger::getInstance().log(LogLevel::INFO, state);
    } else if (level == LogLevel::ERROR) {
        Logger::getInstance().log(LogLevel::ERROR, state);
    } else if (level == LogLevel::WARNING) {
        Logger::getInstance().log(LogLevel::WARNING, state);
    } else if (level == LogLevel::DEBUG) {
        Logger::getInstance().log(LogLevel::DEBUG, state);
    } else if (level == LogLevel::CRITICAL) {
        Logger::getInstance().log(LogLevel::CRITICAL, state);
    }
}

void RobotNamespace::Robot::isMoving()
{
    if (robotStatus.activityState == "MOVIENDO") {
        exceptionAndExecute("El robot está en movimiento. Operación no permitida en este estado.");
        return;
    }
}

/// @brief Envía un comando y espera activamente una respuesta.
static std::string sendAndReceive(ComunicatorPort::ISerialCommunicator& serial, const std::string& command, int time) {
    serial.sendMessage(command);
    
    std::string full_response;
    // Intentamos leer varias veces, ya que la respuesta puede llegar en fragmentos.
    while (true)
    {
        full_response = serial.reciveMessage(time);
        if (full_response.find("OK") != std::string::npos) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            break;
        }
    }
    size_t error_pos = 0;
    if ((error_pos = full_response.find("ERROR")) != std::string::npos){
        // Si encontramos "ERROR", lanzamos la excepción solo con el mensaje a partir de ese punto.
        full_response.erase(std::remove(full_response.begin(), full_response.end(), '\n'), full_response.end());
        full_response.erase(std::remove(full_response.begin(), full_response.end(), '\r'), full_response.end());
        throw RobotException(full_response.substr(error_pos-4));
    }

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

void RobotNamespace::Robot::executeMoviment(const Position& position, double speed){
    isMoving();
    if (robotStatus.isConnected && robotStatus.areMotorsEnabled) {
        // 1. Generar el comando G-Code a partir de la posición y velocidad.
        std::string gcodeCommand;
        if(position.x == 0 && position.y == 0 && position.z == 0) {
            logAndExecuteState(LogLevel::INFO, "[Robot] Moviendo al origen (0,0,0).");
            robotStatus.activityState = "ORIGEN";
            gcodeCommand = "G28\r\n";
        } else {
            if (speed != 2000.0){
                gcodeCommand = GCodeNamespace::GCode::generateMoveCommand(position.x, position.y, position.z, speed);
                Logger::getInstance().log(LogLevel::INFO, "[Robot] Generado G-Code: \"" + gcodeCommand + "\"");
            } else {
                gcodeCommand = GCodeNamespace::GCode::generateMoveCommand(position.x, position.y, position.z);
                Logger::getInstance().log(LogLevel::INFO, "[Robot] Generado G-Code (velocidad por defecto): \"" + gcodeCommand + "\"");
            }
        }
        try {
            // 2. Enviar el comando a través del comunicador serie.
            Logger::getInstance().log(LogLevel::INFO, "[Robot] Enviando comando al puerto serie...");
            std::string response = sendAndReceive(ServiceLocator::getCommunicator(), gcodeCommand + "\r\n", 3); // Mayor timeout para movimientos

            if (!(response.find("ERROR") != std::string::npos)){
                robotStatus.activityState = "MOVIENDO"; // Cambiar estado ANTES de enviar
                Logger::getInstance().log(LogLevel::INFO, "[Robot] Comando de movimiento aceptado por el robot. Respuesta: " + response);
                // NOTA: El robot ahora está físicamente en movimiento.
                // El estado se quedará en "MOVIENDO". Se necesita un mecanismo (hilo, sondeo) para detectar el fin del movimiento.
                // Por ahora, lo cambiaremos a EN_POSICION para que no se bloquee.
                robotStatus.activityState = "EN_POSICION";
                logAndExecuteState(LogLevel::INFO, "[Robot] Movimiento completado.");
            } else {
                exceptionAndExecute(response);
            }
        } catch (const SerialCommunicationException& e) {
            logAndExecuteState(LogLevel::ERROR, "[Robot] Error al enviar comando de movimiento." + std::string(e.what()));
            robotStatus.activityState = "ERROR";
            return; // Salimos si hubo un error al enviar.
        }

    } else {
        exceptionAndExecute("No se puede mover. Asegúrese de que el robot esté conectado y los motores estén habilitados.");
    }
}

void RobotNamespace::Robot::exceptionAndExecute(std::string e){
    setExecuteState(e);
    throw RobotException(e);
}

void RobotNamespace::Robot::initAttributes()
{
}