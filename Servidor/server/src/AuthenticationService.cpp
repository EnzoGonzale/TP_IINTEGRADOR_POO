#include "AuthenticationService.h"
#include "Exceptions.h" // Incluimos nuestras excepciones personalizadas
#include "Logger.h"
#include "bcrypt.h" // Incluimos la librería de hashing
// Constructors/Destructors


AuthenticationServiceNamespace::AuthenticationService::AuthenticationService(DatabaseManagerNamespace::DatabaseManager& dbManager, SessionManager& sessionManager)
    : dbManager_(dbManager), sessionManager_(sessionManager)
{
}

AuthenticationServiceNamespace::AuthenticationService::~AuthenticationService()
{
}

std::string AuthenticationServiceNamespace::AuthenticationService::login(const std::string& username, const std::string& password, const std::string& clientIp) {
    try {
        auto userOpt = dbManager_.findUser(username);
        if (userOpt && userOpt->checkPassword(password)) {
            // Si las credenciales son correctas, creamos una sesión y devolvemos el token.
            return sessionManager_.createSession(*userOpt, clientIp);
        }
        // Si el usuario no existe o la contraseña es incorrecta, lanzamos una excepción.
        throw InvalidCredentialsException("Credenciales inválidas para el usuario '" + username + "'.");
    } catch (const DatabaseException& e) {
        // Si hubo un error en la base de datos, lo relanzamos para que lo maneje una capa superior.
        throw;
    }
}

void AuthenticationServiceNamespace::AuthenticationService::logout(const std::string& token) {
    sessionManager_.endSession(token);
}

std::optional<UserNamespace::User> AuthenticationServiceNamespace::AuthenticationService::validateToken(const std::string& token) const {
    return sessionManager_.getUserByToken(token);
}

bool AuthenticationServiceNamespace::AuthenticationService::createUser(const std::string& username, const std::string& password, UserRole role) {
    // Generamos un hash seguro de la contraseña usando bcrypt.
    std::string passwordHash = bcrypt::generateHash(password);
    if (!dbManager_.addUser(username, passwordHash, role)) {
        throw DatabaseException("No se pudo crear el usuario '" + username + "'. Es posible que ya exista.");
    }
    return true;
}

std::map<std::string, std::pair<UserNamespace::User, std::string>> AuthenticationServiceNamespace::AuthenticationService::getActiveUsersWithIPs() {
    // Ahora que el SessionManager guarda la IP, simplemente devolvemos la información.
    return sessionManager_.getActiveSessions();
}
