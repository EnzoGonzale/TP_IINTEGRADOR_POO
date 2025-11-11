#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "SerialComunicator.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <unistd.h>  // para usleep
#include <algorithm> // para std::remove

// --- Función de Ayuda para la Prueba ---
// Envía un comando y espera activamente una respuesta que contenga "OK".
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


// --- Suite de Pruebas de Integración para SerialComunicator ---
TEST_SUITE("SerialComunicator Integration Test") {
    // Setup shared resources
    static ComunicatorPort::SerialComunicator serial;

    TEST_CASE("Prueba de comandos de control del robot (M3, M5, M17, M18)") {
        ComunicatorPort::SerialComunicator serial;

        // Move configuration to a separate setup function
        if (!serial.isConfigured()) {
            INFO("Configurando puerto /dev/ttyUSB0 a 115200 baudios...");
            serial.config("/dev/ttyUSB0", 115200);
            
            INFO("Esperando reinicio del Arduino (2 segundos)...");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            serial.cleanBuffer();
        }

        // --- Pruebas de Comandos ---
        SUBCASE("Activar Motores (M17)") {
            std::cout << "  [TEST] Enviando M17..." << std::endl;
            std::string response = sendAndReceive(serial, "M17\r\n");
            std::cout << "  [TEST] Respuesta M17: '" << response << "'" << std::endl;
            INFO("Respuesta recibida: '", response, "'"); // Se mostrará solo si el CHECK falla
            CHECK(response.find("OK") != std::string::npos);
        }

        SUBCASE("Activar Efector (M3)") {
            std::cout << "  [TEST] Enviando M3..." << std::endl;
            std::string response = sendAndReceive(serial, "M3\r\n");
            std::cout << "  [TEST] Respuesta M3: '" << response << "'" << std::endl;
            INFO("Respuesta recibida: '", response, "'");
            CHECK(response.find("OK") != std::string::npos);
        }

        SUBCASE("Desactivar Efector (M5)") {
            std::cout << "  [TEST] Enviando M5..." << std::endl;
            std::string response = sendAndReceive(serial, "M5\r\n");
            std::cout << "  [TEST] Respuesta M5: '" << response << "'" << std::endl;
            INFO("Respuesta recibida: '", response, "'");
            CHECK(response.find("OK") != std::string::npos);
        }

        SUBCASE("Desactivar Motores (M18)") {
            std::cout << "  [TEST] Enviando M18..." << std::endl;
            std::string response = sendAndReceive(serial, "M18\r\n");
            std::cout << "  [TEST] Respuesta M18: '" << response << "'" << std::endl;
            INFO("Respuesta recibida: '", response, "'");
            CHECK(response.find("OK") != std::string::npos);
        }
    }
}