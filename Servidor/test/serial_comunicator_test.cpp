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

    TEST_CASE("Prueba de comandos de control del robot (M3, M5, M17, M18)") {
        ComunicatorPort::SerialComunicator serial;

        // --- Configuración ---
        // Esta prueba requiere un Arduino físico conectado a /dev/ttyUSB0
        // y permisos para acceder al puerto (ejecutar con sudo).
        INFO("Configurando puerto /dev/ttyUSB0 a 115200 baudios...");
        REQUIRE_NOTHROW(serial.config("/dev/ttyUSB0", 115200));
        
        // Esperamos a que el Arduino se reinicie tras abrir el puerto.
        INFO("Esperando reinicio del Arduino (2 segundos)...");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        serial.cleanBuffer(); // Limpiamos cualquier mensaje de arranque.

        // --- Pruebas de Comandos ---
        SUBCASE("Activar Motores (M17)") {
            std::cout << "  [TEST] Enviando M17..." << std::endl;
            std::string response = sendAndReceive(serial, "M17\r\n");
            std::cout << "  [TEST] Respuesta M17: '" << response << "'" << std::endl;
            INFO("Respuesta recibida: '", response, "'"); // Se mostrará solo si el CHECK falla
            CHECK(response == "INFO: MOTORS ENABLED\r\nOK\r\n");
        }

        SUBCASE("Activar Efector (M3)") {
            std::cout << "  [TEST] Enviando M3..." << std::endl;
            std::string response = sendAndReceive(serial, "M3\r\n");
            std::cout << "  [TEST] Respuesta M3: '" << response << "'" << std::endl;
            INFO("Respuesta recibida: '", response, "'");
            CHECK(response == "INFO: GRIPPER ON\r\nOK\r\n");
        }

        SUBCASE("Desactivar Efector (M5)") {
            std::cout << "  [TEST] Enviando M5..." << std::endl;
            std::string response = sendAndReceive(serial, "M5\r\n");
            std::cout << "  [TEST] Respuesta M5: '" << response << "'" << std::endl;
            INFO("Respuesta recibida: '", response, "'");
            CHECK(response == "INFO: GRIPPER OFF\r\nOK\r\n");
        }

        SUBCASE("Desactivar Motores (M18)") {
            std::cout << "  [TEST] Enviando M18..." << std::endl;
            std::string response = sendAndReceive(serial, "M18\r\n");
            std::cout << "  [TEST] Respuesta M18: '" << response << "'" << std::endl;
            INFO("Respuesta recibida: '", response, "'");
            CHECK(response == "INFO: MOTORS DISABLED\r\nOK\r\n");
        }
    }
}


// int main(){

//     ComunicatorPort::SerialComunicator serial;

//     // --- Configuración ---
//     std::cout << "Configurando puerto /dev/ttyUSB0 a 115200 baudios..." << std::endl;
//     serial.config("/dev/ttyUSB0", 115200);
//     std::cout << "Esperando reinicio del Arduino (2 segundos)..." << std::endl;
//     std::this_thread::sleep_for(std::chrono::seconds(2));
//     serial.cleanBuffer(); // Limpiamos cualquier mensaje de arranque.
//     std::cout << "Puerto configurado." << std::endl;

//     // --- Pruebas de Comandos ---
//     std::cout << "Enviando M17..." << std::endl;
//     std::cout << sendAndReceive(serial, "M17\r\n") << std::endl;
//     std::cout << "Enviando M3..." << std::endl;
//     std::cout << sendAndReceive(serial, "M3\r\n") << std::endl;
//     std::cout << "Enviando M5..." << std::endl;
//     std::cout << sendAndReceive(serial, "M5\r\n") << std::endl;
//     std::cout << "Enviando M18..." << std::endl;
//     std::cout << sendAndReceive(serial, "M18\r\n") << std::endl;

//     return 0;
// }