#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Robot.h"
#include "ISerialCommunicator.h" // Incluimos la interfaz
#include "ServiceLocator.h"      // Incluimos el Service Locator
#include <iostream>
#include <iomanip>

// --- Mock del Comunicador Serie ---
// Esta clase simula el comportamiento de SerialComunicator para las pruebas
// sin necesidad de hardware real.
class MockSerialCommunicator : public ComunicatorPort::ISerialCommunicator {
public:
    void config(const std::string& port, int speed) override { 
        (void)port; (void)speed; 
        is_configured = true;
    }
    std::string sendMessage(const std::string& message) override {
        last_message_sent = message;
        return "OK";
    }
    std::string reciveMessage(int time) override {
        (void)time;
        if (last_message_sent == "M114\r\n") {
            return "INFO: ABSOLUTE MODE\n"
                   "INFO: CURRENT POSITION: [X:5.66 Y:85.14 Z:69.09 E:0.00]\n"
                   "INFO: MOTORS DISABLED\n"
                   "OK\n";
        }
        return "OK\r\n";
    }
    void cleanBuffer() override {}
    void close() override { is_configured = false; }
    bool isConfigured() const override { return is_configured; }

private:
    bool is_configured = false;
    std::string last_message_sent;
};

TEST_SUITE("Robot Status Parsing") {

    TEST_CASE("Parsing M114 response and checking RobotStatus struct") {
        // 1. Creamos el mock y lo registramos en el ServiceLocator.
        MockSerialCommunicator mockCommunicator;
        ServiceLocator::provide(&mockCommunicator);

        // 2. Creamos el robot. Su constructor usará el mock a través del ServiceLocator.
        RobotNamespace::Robot robot;

        // Simulamos que el robot está conectado para que getStatus() funcione.
        robot.connect(); 

        std::cout << "\n--- Obteniendo y parseando estado del Robot (simulado) ---\n";

        // getStatus() ahora llamará a nuestro sendAndReceive falso y parseará su respuesta.
        RobotStatus status = robot.getStatus();

        // --- Verificaciones con doctest ---
        // Comprobamos que los valores parseados son los correctos.
        CHECK(status.isAbsolute == true);
        CHECK(status.areMotorsEnabled == false);
        CHECK(status.currentPosition.x == doctest::Approx(5.66));
        CHECK(status.currentPosition.y == doctest::Approx(85.14));
        CHECK(status.currentPosition.z == doctest::Approx(69.09));

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