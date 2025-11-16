
#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <string>
#include <map>
#include "User.h"
#include "Robot.h" // Necesario para acceder a los datos del robot
#include <xmlrpc-c/base.hpp> // Necesario para xmlrpc_c::value



/// 
/// class ReportGenerator

class ReportGenerator
{
public:
  // Constructors/Destructors  



  /// 
  /// Empty Constructor
  ReportGenerator();

  /// 
  /// Empty Destructor
  virtual ~ReportGenerator();



  /// 
  /// @return string
  /// @param  robot La instancia del robot para obtener su estado y órdenes.
  /// @param  user El usuario para el cual se genera el reporte (para filtrar órdenes).
  /// @return Un xmlrpc_c::value_struct con toda la información del reporte.
  xmlrpc_c::value generateOperatorReport(const RobotNamespace::Robot& robot, const UserNamespace::User& user);


  /// 
  /// @return string
  /// @param  filters 
  /// @param  robot La instancia del robot para obtener todas las órdenes.
  /// @param  filters Un mapa de filtros (ej. "username", "commandName", "success").
  /// @return Un xmlrpc_c::value_struct con toda la información del reporte de administrador.
  xmlrpc_c::value generateAdminReport(const RobotNamespace::Robot& robot, const std::map<std::string, std::string>& filters);


  /// 
  /// @return string
  /// @param  filters Un mapa de filtros (ej. "username", "level").
  /// @return Un xmlrpc_c::value_struct con el reporte del log.
  xmlrpc_c::value generateLogReport(const std::map<std::string, std::string>& filters);


};

#endif // REPORTGENERATOR_H
