#include "AuthenticationService.h"

// Constructors/Destructors


AuthenticationServiceNamespace::AuthenticationService::AuthenticationService(DatabaseManagerNamespace::DatabaseManager& dbManager)
    : dbManager(dbManager)
{
}

AuthenticationServiceNamespace::AuthenticationService::~AuthenticationService()
{
}
 
std::optional<UserNamespace::User> AuthenticationServiceNamespace::AuthenticationService::authenticate(const std::string& username, const std::string& password) {
    auto userOpt = dbManager.findUser(username);
    if (userOpt && userOpt->checkPassword(password)) {
        return userOpt;
    }
    return std::nullopt;
}

bool AuthenticationServiceNamespace::AuthenticationService::createUser(const std::string& username, const std::string& password, UserRole role) {
    // En un sistema real, aquí se generaría un hash seguro de la contraseña.
    // Por ahora, la guardamos en texto plano como si fuera un hash.
    std::string passwordHash = password;
    return dbManager.addUser(username, passwordHash, role);
}
