
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <list>
#include <optional>
#include "User.h"
#include <sqlite3.h>

namespace DatabaseManagerNamespace
{

/// 
/// class DatabaseManager

class DatabaseManager
{
public:
  // Constructors/Destructors  

  /// @brief Constructor que abre la conexión a la base de datos.
  /// @param dbPath La ruta al archivo de la base de datos SQLite.
  DatabaseManager(const std::string& dbPath);

  /// 
  /// Empty Destructor
  virtual ~DatabaseManager();



  /// 
  /// @return Un objeto User si se encuentra, o std::nullopt si no.
  /// @param  username 
  std::optional<UserNamespace::User> findUser(const std::string& username);

  /// @brief Añade un nuevo usuario a la base de datos.
  /// @param username El nombre del nuevo usuario.
  /// @param passwordHash El hash de la contraseña.
  /// @param role El rol del nuevo usuario.
  /// @return True si el usuario fue creado, false si ya existía o hubo un error.
  bool addUser(const std::string& username, const std::string& passwordHash, UserRole role);

  /// 
  /// @return list<User>
  std::list<UserNamespace::User> getAllUsers()
  {
    return std::list<UserNamespace::User>();
  }

private:
  // Private attributes  

  /// @brief Crea la tabla de usuarios si no existe.
  void createTable();

  sqlite3* db;
  std::string dbPath;

};

} // namespace DatabaseManagerNamespace

#endif // DATABASEMANAGER_H
