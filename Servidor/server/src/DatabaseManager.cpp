#include "DatabaseManager.h"
#include <stdexcept>
#include "Logger.h"
#include "Exceptions.h"
#include "bcrypt.h"


// Constructors/Destructors


DatabaseManagerNamespace::DatabaseManager::DatabaseManager(const std::string& dbPath) : db(nullptr), dbPath(dbPath)
{
    // Usamos sqlite3_open_v2 para ser explícitos sobre la creación del archivo.
    if (sqlite3_open_v2(dbPath.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
        std::string errorMsg = "No se puede abrir la base de datos: ";
        errorMsg += sqlite3_errmsg(db);
        throw DatabaseException(errorMsg);
    } else {
        Logger::getInstance().log(LogLevel::INFO, "[DB] Base de datos abierta en " + dbPath);
        createTable();
    }
}

DatabaseManagerNamespace::DatabaseManager::~DatabaseManager()
{
    if (db) {
        sqlite3_close(db);
        Logger::getInstance().log(LogLevel::INFO, "[DB] Base de datos cerrada.");
    }
}

void DatabaseManagerNamespace::DatabaseManager::createTable() {
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "password_hash TEXT NOT NULL,"
        "role INTEGER NOT NULL);";

    char* errMsg = 0;
    if (sqlite3_exec(db, sql, 0, 0, &errMsg) != SQLITE_OK) {
        std::string error = "Error al crear la tabla: ";
        error += errMsg;
        sqlite3_free(errMsg);
        throw DatabaseException(error);
    }

    // Comprobar si la tabla está vacía para crear el superusuario
    const char* countSql = "SELECT COUNT(*) FROM users;";
    sqlite3_stmt* stmt;
    int userCount = 0;

    if (sqlite3_prepare_v2(db, countSql, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            userCount = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (userCount == 0) {
        Logger::getInstance().log(LogLevel::WARNING, "[DB] La tabla de usuarios está vacía. Creando Administrador Principal...");
        // Reutilizamos la función addUser para crear el usuario por defecto.
        if (addUser("principalAdmin", bcrypt::generateHash("1234"), UserRole::ADMIN)) {
            Logger::getInstance().log(LogLevel::INFO, "[DB] Usuario 'principalAdmin' creado con clave por defecto.");
        } else {
            Logger::getInstance().log(LogLevel::ERROR, "[DB] No se pudo crear el Administrador Principal.");
        }
    }
    Logger::getInstance().log(LogLevel::INFO, "[DB] Tabla de usuarios lista.");  
}

std::optional<UserNamespace::User> DatabaseManagerNamespace::DatabaseManager::findUser(const std::string& username) {
    const char* sql = "SELECT id, password_hash, role FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        return std::nullopt; // Error en la preparación de la consulta
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        UserRole role = static_cast<UserRole>(sqlite3_column_int(stmt, 2));

        sqlite3_finalize(stmt);
        return UserNamespace::User(id, username, passwordHash, role);
    }

    sqlite3_finalize(stmt);
    return std::nullopt; // Usuario no encontrado
}

bool DatabaseManagerNamespace::DatabaseManager::addUser(const std::string& username, const std::string& passwordHash, UserRole role) {
    const char* sql = "INSERT INTO users (username, password_hash, role) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        std::string error = "[DB] Error al preparar la inserción: ";
        error += sqlite3_errmsg(db);
        throw DatabaseException(error);
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, static_cast<int>(role));

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    
    sqlite3_finalize(stmt);

    Logger::getInstance().log(LogLevel::INFO, "[DB] Usuario '" + username + "' " + (success ? "creado exitosamente." : "no pudo ser creado (o ya existe)."));
    
    return success;
}
