
#ifndef AUTHENTICATIONSERVICE_H
#define AUTHENTICATIONSERVICE_H

#include <string>
#include <vector>
#include "DatabaseManager.h"
#include <optional>

#include "User.h"
#include "SessionManager.h"


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
  AuthenticationService(DatabaseManagerNamespace::DatabaseManager& dbManager, SessionManager& sessionManager);

  /// 
  /// Empty Destructor
  virtual ~AuthenticationService();

  /// @brief Crea un nuevo usuario.
  /// @param username El nombre del nuevo usuario.
  /// @param password La contraseña en texto plano.
  /// @param role El rol del nuevo usuario.
  /// @return True si se creó correctamente.
  bool createUser(const std::string& username, const std::string& password, UserRole role);


  std::string login(const std::string& username, const std::string& password, const std::string& clientIp);
  void logout(const std::string& token);
  std::optional<UserNamespace::User> validateToken(const std::string& token) const;


  std::map<std::string, std::pair<UserNamespace::User, std::string>> getActiveUsersWithIPs();

private:
  DatabaseManagerNamespace::DatabaseManager& dbManager_;
  SessionManager& sessionManager_;

};

} // namespace AuthenticationServiceNamespace

#endif // AUTHENTICATIONSERVICE_H
