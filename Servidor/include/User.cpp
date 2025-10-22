#include "User.h"

// Constructors/Destructors


UserNamespace::User::User(int id, const std::string& username, const std::string& passwordHash, UserRole role)
    : id(id), username(username), passwordHash(passwordHash), role(role)
{}

UserNamespace::User::~User()
{
}

int UserNamespace::User::getId() const {
    return id;
}

const std::string& UserNamespace::User::getUsername() const {
    return username;
}

bool UserNamespace::User::checkPassword(const std::string& password) const {
    // Por ahora, una simple comparación. En un sistema real, aquí se compararía el hash.
    return password == passwordHash;
}

UserRole UserNamespace::User::getRole() const {
    return role;
}

bool UserNamespace::User::hasRole(UserRole roleToCheck) const {
    return this->role == roleToCheck;
}
