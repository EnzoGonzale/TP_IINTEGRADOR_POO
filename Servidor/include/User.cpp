#include "User.h"
#include "bcrypt.h"

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
    // Compara la contraseña en texto plano con el hash guardado en la BD.
    return bcrypt::validatePassword(password, this->passwordHash);
}

UserRole UserNamespace::User::getRole() const {
    return role;
}

bool UserNamespace::User::hasRole(UserRole roleToCheck) const {
    return this->role == roleToCheck;
}
