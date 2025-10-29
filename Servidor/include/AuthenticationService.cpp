#include "AuthenticationService.h"
#include "Exceptions.h" // Incluimos nuestras excepciones personalizadas
#include "Logger.h"
// Constructors/Destructors


AuthenticationServiceNamespace::AuthenticationService::AuthenticationService(DatabaseManagerNamespace::DatabaseManager& dbManager)
    : dbManager(dbManager)
{
}

AuthenticationServiceNamespace::AuthenticationService::~AuthenticationService()
{
}
 
std::optional<UserNamespace::User> AuthenticationServiceNamespace::AuthenticationService::authenticate(const std::string& username, const std::string& password) {
    try {
        auto userOpt = dbManager.findUser(username);
        if (userOpt && userOpt->checkPassword(password)) {
            return userOpt;
        }
        // Si el usuario no existe o la contraseña es incorrecta, lanzamos una excepción.
        throw InvalidCredentialsException();
    } catch (const DatabaseException& e) {
        // Si hubo un error en la base de datos, lo relanzamos para que lo maneje una capa superior.
        throw;
    }
}

bool AuthenticationServiceNamespace::AuthenticationService::createUser(const std::string& username, const std::string& password, UserRole role) {
    // En un sistema real, aquí se generaría un hash seguro de la contraseña.
    // Por ahora, la guardamos en texto plano como si fuera un hash.
    std::string passwordHash = password;
    if (!dbManager.addUser(username, passwordHash, role)) {
        throw DatabaseException("No se pudo crear el usuario '" + username + "'. Es posible que ya exista.");
    }
    return true;
}
