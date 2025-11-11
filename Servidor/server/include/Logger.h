
#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <optional> // Para std::optional
#include "FileManager.h"



/// @brief Define los diferentes niveles de severidad para los mensajes de log.
enum class LogLevel {
    /// @brief Mensajes de diagnóstico detallados, útiles para depuración.
    /// Ejemplo: "Valor de la variable X es 10 en la función Y".
    DEBUG, 
    /// @brief Mensajes informativos sobre el progreso normal de la aplicación.
    /// Ejemplo: "Servidor iniciado en el puerto 8080", "Usuario 'admin' ha iniciado sesión".
    INFO,    
    /// @brief Indica una situación inesperada que no es un error, pero que podría serlo.
    /// Ejemplo: "La conexión a la base de datos tardó más de lo esperado".
    WARNING, 
    /// @brief Indica un error en una operación específica que no detiene la aplicación.
    /// Ejemplo: "No se pudo crear el usuario 'test' porque ya existe".
    ERROR,   
    /// @brief Un error grave que probablemente cause la terminación de la aplicación.
    /// Ejemplo: "No se pudo abrir la base de datos. El servidor no puede continuar".
    CRITICAL 
};

/// 
/// class Logger

class Logger
{
public:
    // Método estático para obtener la única instancia de la clase
    static Logger& getInstance();

    // Eliminamos el constructor de copia y el operador de asignación
    Logger(const Logger&) = delete;
    void operator=(const Logger&) = delete;

    // Método principal para registrar un mensaje
    void log(LogLevel level, 
             const std::string& message, 
             const std::optional<std::string>& user = std::nullopt, 
             const std::optional<std::string>& node = std::nullopt);

private:
    // Constructor y destructor privados para asegurar que no se creen instancias externamente
    Logger();
    ~Logger();
    std::string levelToString(LogLevel level);

    FileNamespace::FileManager fileManager_;
};

#endif // LOGGER_H
