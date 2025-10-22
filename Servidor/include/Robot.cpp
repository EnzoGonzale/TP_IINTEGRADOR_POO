#include "Robot.h"
#include <iostream>
#include "GCode.h" // Incluimos la clase GCode para usar su funcionalidad
#include <stdexcept>  // Para std::runtime_error
#include <thread>     // Para std::this_thread::sleep_for
#include <unistd.h> // Para usleep
#include <algorithm>  // Para std::remove

// --- Declaración de la nueva función privada ---
std::string sendAndReceive(ComunicatorPort::SerialComunicator& serial, const std::string& command);


RobotNamespace::Robot::Robot() : isConnected(false),
                 areMotorsEnabled(false),
                 activityState("IDLE"),
                 serialCommunicator() // Llama al constructor por defecto de SerialComunicator
{
}

RobotNamespace::Robot::~Robot()
{
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
        activityState = "CONECTADO";
        std::cout << "[Robot] Conexión establecida." << std::endl;
    } else {
        std::cout << "[Robot] Advertencia: El robot ya se encuentra conectado." << std::endl;
    }
}

void RobotNamespace::Robot::disconnect() {
    if (isConnected) {
        std::cout << "[Robot] Desconectando..." << std::endl;
        serialCommunicator.close();
        isConnected = false;
        areMotorsEnabled = false; // Al desconectar, los motores se apagan
        activityState = "DESCONECTADO";
        std::cout << "[Robot] Conexión cerrada." << std::endl;
    } else {
        std::cout << "[Robot] Advertencia: El robot ya se encuentra desconectado." << std::endl;
    }
}

void RobotNamespace::Robot::enableMotors() {
    if (activityState == "MOVIENDO") {
        std::cout << "[Robot] Error: No se pueden modificar los motores mientras el robot está en movimiento." << std::endl;
        return;
    }
    if (isConnected && !areMotorsEnabled) {
        areMotorsEnabled = true;
        std::string response = sendAndReceive(serialCommunicator, "M17\r\n");
        std::cout << "[Robot] Respuesta de M17: " << (response.empty() ? "[ninguna]" : response) << std::endl;
        std::cout << "[Robot] Motores activados." << std::endl;
    } else if (!isConnected) {
        std::cout << "[Robot] Error: No se pueden activar los motores, el robot no está conectado." << std::endl;
    }
}

void RobotNamespace::Robot::disableMotors() {
    if (activityState == "MOVIENDO") {
        std::cout << "[Robot] Error: No se pueden modificar los motores mientras el robot está en movimiento." << std::endl;
        return;
    }
    if (areMotorsEnabled) {
        areMotorsEnabled = false;
        std::string response = sendAndReceive(serialCommunicator, "M18\r\n");
        std::cout << "[Robot] Motores desactivados." << std::endl;
        std::cout << "[Robot] Respuesta de M18: " << (response.empty() ? "[ninguna]" : response) << std::endl;
    }
}

void RobotNamespace::Robot::moveTo(const Position& position, double speed) {
    if (activityState == "MOVIENDO") {
        std::cout << "[Robot] Error: Ya hay un movimiento en curso." << std::endl;
        return;
    }
    if (isConnected && areMotorsEnabled) {
        // 1. Generar el comando G-Code a partir de la posición y velocidad.
        std::string gcodeCommand;
        if(position.x == 0 && position.y == 0 && position.z == 0) {
            std::cout << "[Robot] Moviendo al origen (0, 0, 0) " << std::endl;
            activityState = "ORIGEN";
            gcodeCommand = "G24\r";
        } else {
            gcodeCommand = GCodeNamespace::GCode::generateMoveCommand(position.x, position.y, position.z, speed);
            std::cout << "[Robot] Generado G-Code: \"" << gcodeCommand << "\"" << std::endl;
        }
        try {
            // 2. Enviar el comando a través del comunicador serie.
            std::cout << "[Robot] Enviando comando al puerto serie..." << std::endl;
            currentPosition = position;
            activityState = "MOVIENDO"; // Cambiar estado ANTES de enviar
            std::string response = sendAndReceive(serialCommunicator, gcodeCommand + "\r\n"); // Mayor timeout para movimientos
            std::cout << "[Robot] Comando de movimiento aceptado por el robot. Respuesta: " << response << std::endl;
            // NOTA: El robot ahora está físicamente en movimiento.
            // El estado se quedará en "MOVIENDO". Se necesita un mecanismo (hilo, sondeo) para detectar el fin del movimiento.
            // Por ahora, lo cambiaremos a EN_POSICION para que no se bloquee.
            activityState = "EN_POSICION";
        } catch (const std::runtime_error& e) {
            std::cerr << "[Robot] Error al enviar comando de movimiento: " << e.what() << std::endl;
            activityState = "ERROR";
            return; // Salimos si hubo un error al enviar.
        }

        // Para ver el funcionamiento de la comunicacion serie
        // La respuesta ya se gestionó en sendCommandAndWaitForResponse

    } else {
        std::cout << "[Robot] Error: No se puede mover. Verifique la conexión y el estado de los motores." << std::endl;
    }
}

void RobotNamespace::Robot::setEffector(bool active) {
    if (activityState == "MOVIENDO") {
        std::cout << "[Robot] Error: No se puede operar el efector mientras el robot está en movimiento." << std::endl;
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
        std::cout << "[Robot] Efector final " << (active ? "activado." : "desactivado.") << std::endl;
    }
}

void RobotNamespace::Robot::initAttributes()
{
}

/// @brief Envía un comando y espera activamente una respuesta.
std::string sendAndReceive(ComunicatorPort::SerialComunicator& serial, const std::string& command) {
    serial.sendMessage(command);
    
    std::string full_response;
    // Intentamos leer varias veces, ya que la respuesta puede llegar en fragmentos.
    full_response = serial.reciveMessage();

    // Limpiamos saltos de línea y retornos de carro para una comparación limpia.
    // full_response.erase(std::remove(full_response.begin(), full_response.end(), '\n'), full_response.end());
    // full_response.erase(std::remove(full_response.begin(), full_response.end(), '\r'), full_response.end());

    return full_response; // Devolvemos lo que se haya recibido, que podría estar vacío.
}
