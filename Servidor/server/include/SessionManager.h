#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <string>
#include <map>
#include <optional>
#include <mutex>
#include "User.h"

/// @brief Gestiona las sesiones de usuario activas basadas en tokens.
/// Esta clase es segura para su uso en entornos multihilo.
class SessionManager {
public:
    SessionManager() = default;

    /// @brief Crea una nueva sesión para un usuario y genera un token único.
    /// @param user El objeto User que ha sido autenticado.
    /// @param ip_address La dirección IP desde la que se conecta el usuario.
    /// @return Un string que representa el token de sesión.
    std::string createSession(const UserNamespace::User& user, const std::string& ip_address);

    /// @brief Valida un token y devuelve el usuario asociado si es válido.
    /// @param token El token de sesión a validar.
    /// @return Un std::optional que contiene el objeto User si el token es válido, o está vacío si no.
    std::optional<UserNamespace::User> getUserByToken(const std::string& token) const;

    /// @brief Finaliza una sesión, invalidando el token.
    /// @param token El token de sesión a eliminar.
    void endSession(const std::string& token);

    /// @brief Devuelve una copia de todas las sesiones activas.
    /// @return Un mapa de token a un par de {Usuario, IP}.
    std::map<std::string, std::pair<UserNamespace::User, std::string>> getActiveSessions() const;

private:
    /// @brief Genera una cadena de texto aleatoria y segura para ser usada como token.
    /// @return Un token de 32 caracteres hexadecimales.
    std::string generateToken();

    mutable std::mutex sessionMutex_; // `mutable` para poder usarlo en métodos const
    std::map<std::string, std::pair<UserNamespace::User, std::string>> activeSessions_;
};

#endif // SESSIONMANAGER_H