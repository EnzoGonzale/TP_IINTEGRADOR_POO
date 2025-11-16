#include "SessionManager.h"
#include <random>
#include <sstream>
#include <iomanip>

std::string SessionManager::createSession(const UserNamespace::User& user, const std::string& ip_address) {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    std::string token = generateToken();
    // Nos aseguramos de que el token sea único (muy improbable que no lo sea)
    while (activeSessions_.count(token)) {
        token = generateToken();
    }
    activeSessions_.insert({token, {user, ip_address}});
    return token;
}

std::optional<UserNamespace::User> SessionManager::getUserByToken(const std::string& token) const {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    auto it = activeSessions_.find(token);
    if (it != activeSessions_.end()) {
        return it->second.first; // Devuelve solo el objeto User del par
    }
    return std::nullopt; // Token no encontrado
}

void SessionManager::endSession(const std::string& token) {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    activeSessions_.erase(token);
}

std::map<std::string, std::pair<UserNamespace::User, std::string>> SessionManager::getActiveSessions() const {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    return activeSessions_;
}

std::string SessionManager::generateToken() {
    // Generador de números aleatorios seguro
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    // Generamos dos números de 64 bits para tener 128 bits de aleatoriedad
    uint64_t part1 = dis(gen);
    uint64_t part2 = dis(gen);

    // Convertimos a una cadena hexadecimal de 32 caracteres
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << part1
       << std::setw(16) << std::setfill('0') << part2;

    return ss.str();
}