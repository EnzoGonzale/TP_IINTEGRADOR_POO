#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Robot.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>      // Para expresiones regulares


// --- Stub de la función sendAndReceive ---
// Como este es un test unitario para la lógica de parseo, no necesitamos la
// comunicación serie real. Creamos una función "falsa" (stub) para que el
// código compile, ya que Robot.cpp la necesita. La hacemos 'static' para evitar conflictos.
static std::string sendAndReceive(ComunicatorPort::SerialComunicator& serial, const std::string& command, int time) {
    // No usamos los parámetros en este test.
    (void)serial;
    (void)command;
    (void)time;
    // Devolvemos la respuesta que queremos simular.
    return "INFO: ABSOLUTE MODE\n"
           "INFO: CURRENT POSITION: [X:5.66 Y:85.14 Z:69.09 E:0.00]\n"
           "INFO: MOTORS DISABLED\n"
           "INFO: FAN ENABLED\n"
           "OK\n";
}


TEST_SUITE("Robot Status Parsing") {

    TEST_CASE("Parsing M114 response and checking RobotStatus struct") {
        RobotNamespace::Robot robot;

        // Simulamos que el robot está conectado para que getStatus() funcione.
        robot.connect(); 
        robot.enableMotors();

        std::cout << "\n--- Obteniendo y parseando estado del Robot (simulado) ---\n";
        Position pos0;
        pos0.x = 0;
        pos0.y = 0;
        pos0.z = 0;
        robot.moveTo(pos0); // Solo para cambiar el estado interno

        Position pos;
        pos.x = 100;
        pos.y = 100;
        pos.z = 100;
        robot.moveTo(pos); // Solo para cambiar el estado interno

        // getStatus() ahora llamará a nuestro sendAndReceive falso y parseará su respuesta.
        RobotStatus status = robot.getStatus();

        // --- Verificaciones con doctest ---
        // Comprobamos que los valores parseados son los correctos.
        CHECK(status.isAbsolute == true);
        CHECK(status.areMotorsEnabled == true);
        CHECK(status.currentPosition.x == doctest::Approx(100));
        CHECK(status.currentPosition.y == doctest::Approx(100));
        CHECK(status.currentPosition.z == doctest::Approx(100));

        // --- Impresión por consola para inspección visual ---
        std::cout << "\n--- Contenido de la estructura RobotStatus ---\n";
        std::cout << std::boolalpha; // Para imprimir 'true'/'false' en lugar de '1'/'0'
        std::cout << "Conectado: " << status.isConnected << std::endl;
        std::cout << "Estado de Actividad: " << status.activityState << std::endl;
        std::cout << "Motores Activados: " << status.areMotorsEnabled << std::endl;
        std::cout << "Modo de Coordenadas: " << (status.isAbsolute ? "ABSOLUTO" : "RELATIVO") << std::endl;
        std::cout << "Posición Actual: (" 
                  << std::fixed << std::setprecision(2) << status.currentPosition.x << ", " 
                  << status.currentPosition.y << ", " 
                  << status.currentPosition.z << ")" << std::endl;
        std::cout << "---------------------------------------------\n" << std::endl;

        // Desconectamos para limpiar.
        robot.disconnect();
    }
}