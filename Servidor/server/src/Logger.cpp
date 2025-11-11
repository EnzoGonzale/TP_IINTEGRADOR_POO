#include "Logger.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex> // Para std::mutex y std::lock_guard

// Constructors/Destructors

Logger& Logger::getInstance() {
    // La instancia se crea la primera vez que se llama a este método
    // y se destruye automáticamente al final del programa.
    static Logger instance;
    return instance;
}

Logger::Logger() {
    // El constructor del Logger ahora le pide al FileManager que abra el archivo.
    if (!fileManager_.open("application.csv", true)) {
        // Si no se puede abrir, lo notificamos por la consola de errores.
        std::cerr << "CRITICAL: No se pudo abrir el archivo de log 'application.csv'." << std::endl;
    }
    // Mensaje inicial al crear el logger por primera vez.
    log(LogLevel::INFO, "Logger inicializado.");
}

Logger::~Logger() {
    log(LogLevel::INFO, "Logger finalizado.");
    // El destructor del Logger le pide al FileManager que cierre el archivo.
    fileManager_.close();
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
       << (user.has_value() ? ", "+*user : "") // Si no hay usuario, ponemos ""
       << (node.has_value() ? ", "+*node : "");   // Si no hay nodo, ponemos ""

    std::string log_line = ss.str() + "\n";

    // Delegamos la escritura al FileManager.
    if (fileManager_.isOpen()) {
        fileManager_.write(log_line);
        fileManager_.flush(); // Aseguramos que se escriba en disco inmediatamente.
        std::cout << log_line; // También mostramos el log por la consola
    } else {
        // Si el archivo no está abierto, mostramos el log por la consola como último recurso.
        std::cerr << "LOG_FALLBACK: " << log_line;
    }
}
