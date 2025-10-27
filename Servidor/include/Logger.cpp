#include "Logger.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

// Constructors/Destructors

Logger& Logger::getInstance() {
    // La instancia se crea la primera vez que se llama a este método
    // y se destruye automáticamente al final del programa.
    static Logger instance;
    return instance;
}

Logger::Logger()
{
    // Mensaje inicial al crear el logger por primera vez.
    log(LogLevel::INFO, "Logger inicializado.");
}

Logger::~Logger()
{
    log(LogLevel::INFO, "Logger finalizado.");
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:    return "DEBUG";
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARNING";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, 
                 const std::string& message, 
                 const std::optional<std::string>& user, 
                 const std::optional<std::string>& node) {
    // Obtener la fecha y hora actual
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d | %H:%M:%S");

    // Formato CSV: timestamp, level, message, user, node
    ss << ", " << levelToString(level) 
       << ", " << message
       << ", " << (user.has_value() ? *user : "SYSTEM") // Si no hay usuario, ponemos "SYSTEM"
       << ", " << (node.has_value() ? *node : "N/A");   // Si no hay nodo, ponemos "N/A"

    std::string log_line = ss.str() + "\n";

    // Escribir en el archivo de log (application.json) en modo "append"
    file_.write("application.csv", log_line, true);
}
