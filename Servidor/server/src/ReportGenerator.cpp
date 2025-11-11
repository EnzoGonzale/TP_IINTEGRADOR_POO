#include "ReportGenerator.h"
#include <vector>
#include <algorithm> // Para std::find
#include <iostream> // Para std::cout
#include <sstream> // Para std::stringstream
#include "FileManager.h"

// Constructors/Destructors


ReportGenerator::ReportGenerator()
{
}

ReportGenerator::~ReportGenerator()
{
}

// Methods

xmlrpc_c::value ReportGenerator::generateOperatorReport(const RobotNamespace::Robot& robot, const UserNamespace::User& user) {
    std::map<std::string, xmlrpc_c::value> reportMap;

    // 1. Información general del robot
    reportMap["isConnected"] = xmlrpc_c::value_boolean(robot.getRobotStatus().isConnected);
    reportMap["areMotorsEnabled"] = xmlrpc_c::value_boolean(robot.getRobotStatus().areMotorsEnabled);
    reportMap["activityState"] = xmlrpc_c::value_string(robot.getRobotStatus().activityState);
    // reportMap["connectionStartTime"] = xmlrpc_c::value_string(robot.getConnectionStartTime());

    // 2. Posición actual
    std::map<std::string, xmlrpc_c::value> positionMap;
    positionMap["x"] = xmlrpc_c::value_double(robot.getRobotStatus().currentPosition.x);
    positionMap["y"] = xmlrpc_c::value_double(robot.getRobotStatus().currentPosition.y);
    positionMap["z"] = xmlrpc_c::value_double(robot.getRobotStatus().currentPosition.z);
    reportMap["position"] = xmlrpc_c::value_struct(positionMap);

    // 3. Listado de órdenes filtradas por usuario y conteo de errores
    std::vector<xmlrpc_c::value> ordersVector;
    int errorCount = 0;
    for (const auto& order : robot.getLastOrders()) {
        if (order.username == user.getUsername()) {
            std::map<std::string, xmlrpc_c::value> orderMap;
            orderMap["timestamp"] = xmlrpc_c::value_string(order.timestamp);
            orderMap["command"] = xmlrpc_c::value_string(order.commandName);
            orderMap["details"] = xmlrpc_c::value_string(order.details);
            orderMap["success"] = xmlrpc_c::value_string(order.success);
            ordersVector.push_back(xmlrpc_c::value_struct(orderMap));

            // Contar órdenes con error (asumiendo que "ERROR" en la respuesta indica un error)
            if (order.success.find("ERROR") != std::string::npos) {
                errorCount++;
            }
        }
    }
    reportMap["orders"] = xmlrpc_c::value_array(ordersVector);
    reportMap["orderCount"] = xmlrpc_c::value_int(ordersVector.size());
    reportMap["errorOrderCount"] = xmlrpc_c::value_int(errorCount);

    return xmlrpc_c::value_struct(reportMap);
}

xmlrpc_c::value ReportGenerator::generateAdminReport(const RobotNamespace::Robot& robot, const std::map<std::string, std::string>& filters) {
    std::map<std::string, xmlrpc_c::value> reportMap;
    std::vector<xmlrpc_c::value> ordersVector;

    const auto& allOrders = robot.getLastOrders();

    for (const auto& order : allOrders) {
        bool match = true;

        // Aplicar filtros
        if (filters.count("username")) {
            if (order.username != filters.at("username")) {
                match = false;
            }
        }
        if (filters.count("resultado")) {
            // Buscamos si el texto del filtro está contenido en la respuesta
            if (order.success.find(filters.at("resultado")) == std::string::npos) {
                match = false;
            }
        }

        if (match) {
            std::map<std::string, xmlrpc_c::value> orderMap;
            orderMap["timestamp"] = xmlrpc_c::value_string(order.timestamp);
            orderMap["username"] = xmlrpc_c::value_string(order.username); // Añadimos el usuario
            orderMap["command"] = xmlrpc_c::value_string(order.commandName);
            orderMap["details"] = xmlrpc_c::value_string(order.details);
            orderMap["success"] = xmlrpc_c::value_string(order.success);
            ordersVector.push_back(xmlrpc_c::value_struct(orderMap));
        }
    }

    reportMap["orders"] = xmlrpc_c::value_array(ordersVector);
    return xmlrpc_c::value_struct(reportMap);
}


xmlrpc_c::value ReportGenerator::generateLogReport(const std::map<std::string, std::string>& filters) {
    std::map<std::string, xmlrpc_c::value> reportMap;
    std::vector<xmlrpc_c::value> logsVector;

    FileNamespace::FileManager fileManager;
    std::string fileContent = fileManager.read("application.csv");

    std::stringstream ss(fileContent);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;

        std::stringstream lineStream(line);
        std::string timestamp, level, message, user, node;
        
        // Parseo simple de CSV. Asume que no hay comas en el mensaje.
        std::getline(lineStream, timestamp, ',');
        std::getline(lineStream, level, ',');
        std::getline(lineStream, message, ',');
        std::getline(lineStream, user, ',');
        std::getline(lineStream, node);

        // Limpiar espacios en blanco al inicio de los campos
        level.erase(0, level.find_first_not_of(" \t\n\r"));
        user.erase(0, user.find_first_not_of(" \t\n\r"));

        bool match = true;
        if (filters.count("username")) {
            if (user != filters.at("username")) {
                match = false;
            }
        }
        if (filters.count("level")) {
            if (level != filters.at("level")) {
                match = false;
            }
        }

        if (match) {
            std::map<std::string, xmlrpc_c::value> logEntryMap;
            logEntryMap["timestamp"] = xmlrpc_c::value_string(timestamp);
            logEntryMap["level"] = xmlrpc_c::value_string(level);
            logEntryMap["message"] = xmlrpc_c::value_string(message);
            logEntryMap["user"] = xmlrpc_c::value_string(user);
            logEntryMap["node"] = xmlrpc_c::value_string(node);
            logsVector.push_back(xmlrpc_c::value_struct(logEntryMap));
        }
    }

    reportMap["logs"] = xmlrpc_c::value_array(logsVector);
    return xmlrpc_c::value_struct(reportMap);
}


// Accessor methods



// Other methods



