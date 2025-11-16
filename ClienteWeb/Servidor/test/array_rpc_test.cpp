#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <xmlrpc-c/client_simple.hpp>
#include <iostream>
#include <vector>
#include <string>

TEST_SUITE("RPC Array Test") {

    TEST_CASE("Test sending a G-code array to addTask") {
        xmlrpc_c::clientSimple rpcClient;
        std::string const serverUrl("http://localhost:8080/RPC2");
        xmlrpc_c::value result;

        // --- Datos de prueba ---
        std::string const username = "principalAdmin";
        std::string const password = "1234";
        std::string const taskId = "test_array_task";
        std::string const taskName = "TareaDePruebaConArray";

        // 1. Crear el array de comandos G-code
        std::vector<xmlrpc_c::value> gcodeCommands;
        gcodeCommands.push_back(xmlrpc_c::value_string("G28")); // Ir a home
        gcodeCommands.push_back(xmlrpc_c::value_string("G1 X10 Y10 Z10"));
        gcodeCommands.push_back(xmlrpc_c::value_string("M3")); // Activar efector

        // 2. Envolver el vector en los tipos correctos de xmlrpc-c
        xmlrpc_c::value_array gcodeArray(gcodeCommands);

        // 3. Crear un paramList y añadir todos los argumentos
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string(username));
        params.add(xmlrpc_c::value_string(password));
        params.add(xmlrpc_c::value_string(taskId));
        params.add(xmlrpc_c::value_string(taskName));
        params.add(gcodeArray); // Añadir el xmlrpc_c::value_array directamente

        // 4. Llamar al método RPC 'robot.addTask' usando la sobrecarga con paramList
        INFO("Llamando a robot.addTask para crear una nueva tarea...");
        REQUIRE_NOTHROW(
            rpcClient.call(serverUrl, "robot.addTask", params, &result)
        );

        // 5. Verificar que la llamada fue exitosa
        CHECK(xmlrpc_c::value_boolean(result) == true);

        // 6. (Verificación extra) Listar las tareas y comprobar que la nueva existe
        INFO("Llamando a robot.listTasks para verificar la creación...");
        xmlrpc_c::value listResult; // <<--- Usamos una nueva variable para el resultado
        xmlrpc_c::paramList listParams;
        listParams.add(xmlrpc_c::value_string(username));
        listParams.add(xmlrpc_c::value_string(password));
        REQUIRE_NOTHROW(
            rpcClient.call(serverUrl, "robot.listTasks", listParams, &listResult)
        );

        xmlrpc_c::value_array const tasksList(listResult);
        std::vector<xmlrpc_c::value> const tasksVector(tasksList.vectorValueValue());

        bool found = false;
        for (const auto& taskValue : tasksVector) {
            std::map<std::string, xmlrpc_c::value> const taskMap{xmlrpc_c::value_struct(taskValue)};
            std::string id = xmlrpc_c::value_string(taskMap.at("id"));
            if (id == taskId) {
                found = true;
                // También podríamos verificar el nombre y la descripción si quisiéramos
                std::string name = xmlrpc_c::value_string(taskMap.at("name"));
                CHECK(name == taskName);
                break;
            }
        }

        CHECK_MESSAGE(found, "La tarea recién creada no se encontró en la lista de tareas del servidor.");
    }
}