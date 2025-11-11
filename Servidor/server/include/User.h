
#ifndef USER_H
#define USER_H

#include <string>
#include "UserRole.h"


namespace UserNamespace {

/// 
/// class User

class User
{
public:
  // Constructors/Destructors  

  /// 
  /// @brief Constructor para inicializar un objeto User.
  /// @param id El ID único del usuario.
  /// @param username El nombre de usuario.
  /// @param passwordHash El hash de la contraseña.
  /// @param role El rol del usuario (ADMIN u OPERATOR).
  User(int id, const std::string& username, const std::string& passwordHash, UserRole role);

  /// 
  /// Empty Destructor
  virtual ~User();

  // --- Getters Públicos ---

  /// @brief Obtiene el ID del usuario.
  int getId() const;

  /// @brief Obtiene el nombre de usuario.
  const std::string& getUsername() const;

  /// @brief Comprueba si la contraseña proporcionada coincide con el hash almacenado.
  /// @param password La contraseña a verificar.
  /// @return True si la contraseña es correcta, false en caso contrario.
  bool checkPassword(const std::string& password) const;

  /// @brief Obtiene el rol del usuario.
  UserRole getRole() const;

  /// @brief Comprueba si el usuario tiene un rol específico.
  /// @param roleToCheck El rol a comprobar.
  bool hasRole(UserRole roleToCheck) const;

private:
  // Private attributes  

  int id;
  std::string username;
  std::string passwordHash;
  UserRole role;

};

} // namespace UserNamespace

#endif // USER_H
