
#ifndef AUTHENTICATIONSERVICE_H
#define AUTHENTICATIONSERVICE_H

#include <string>
#include <vector>
#include "DatabaseManager.h"
#include <optional>

#include "User.h"


namespace AuthenticationServiceNamespace
{

/// 
/// class AuthenticationService

class AuthenticationService
{
public:
  // Constructors/Destructors  



  /// 
  /// @brief Constructor que recibe sus dependencias.
  AuthenticationService(DatabaseManagerNamespace::DatabaseManager& dbManager);

  /// 
  /// Empty Destructor
  virtual ~AuthenticationService();



  /// 
  /// @return Un objeto User si la autenticación es exitosa, o std::nullopt si falla.
  /// @param  username 
  /// @param  password 
  std::optional<UserNamespace::User> authenticate(const std::string& username, const std::string& password);

  /// @brief Crea un nuevo usuario.
  /// @param username El nombre del nuevo usuario.
  /// @param password La contraseña en texto plano.
  /// @param role El rol del nuevo usuario.
  /// @return True si se creó correctamente.
  bool createUser(const std::string& username, const std::string& password, UserRole role);

private:
  DatabaseManagerNamespace::DatabaseManager& dbManager;

};

} // namespace AuthenticationServiceNamespace

#endif // AUTHENTICATIONSERVICE_H
