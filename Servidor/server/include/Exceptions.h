#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>
#include "Robot.h"
#include "Logger.h"

/// @brief Clase base para todas las excepciones personalizadas de la aplicación.
/// Hereda de std::runtime_error para ser compatible con los bloques catch estándar.
/// Se encarga de registrar automáticamente la excepción en el Logger como CRITICAL.
class AppException : public std::runtime_error {
public:
    /// @brief Constructor que toma un mensaje y lo registra.
    /// @param message El mensaje de error descriptivo.
    explicit AppException(const std::string& message)
        : std::runtime_error(message) {
        // Cada vez que se crea una excepción de nuestra aplicación,
        // se registra automáticamente como CRITICAL en el logger.
        Logger::getInstance().log(LogLevel::CRITICAL, message);
    }
};

// --- Excepciones de Autenticación ---

class AuthenticationException : public AppException {
public:
    explicit AuthenticationException(const std::string& message)
        : AppException("Authentication Error: " + message) {}
};

class InvalidCredentialsException : public AuthenticationException {
public:
    explicit InvalidCredentialsException(const std::string& message = "Invalid username or password.")
        : AuthenticationException(message) {}
};

class PermissionDeniedException : public AuthenticationException {
public:
    explicit PermissionDeniedException(const std::string& message = "Permission denied for this operation.")
        : AuthenticationException(message) {}
};

// --- Excepciones de Comunicación ---

class SerialCommunicationException : public AppException {
public:
    explicit SerialCommunicationException(const std::string& message)
        : AppException("[Serial Communicator] Error: " + message) {}
};

// --- Excepciones de Base de Datos ---

class DatabaseException : public AppException {
public:
    explicit DatabaseException(const std::string& message)
        : AppException("[Database] Error: " + message) {}
};

// --- Excepciones del Robot ---

class RobotException : public AppException {
public:
    explicit RobotException(const std::string& message)
        : AppException(message) {}
};

#endif // EXCEPTIONS_H