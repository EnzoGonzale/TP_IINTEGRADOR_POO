
#ifndef ROBOT_H
#define ROBOT_H

#include <string>
#include <any>
#include <list>
#include "Position.h"

// --- Definiciones de Platzhalter ---
// Estas definiciones deben moverse a sus propios archivos .h a medida que las desarrolles.
struct RobotStatus {
  bool isConnected = false;
  std::string activityState = "DESCONOCIDO";
  bool areMotorsEnabled = false;
  Position currentPosition;
  bool isAbsolute = true; // Por defecto, asumimos modo absoluto
  // ... otros campos de estado
};

#include "Position.h"
#include "GCode.h"
#include "Logger.h"

/// @brief Representa una orden ejecutada por el robot.
struct Order {
  std::string timestamp;
  std::string username;
  std::string commandName;
  std::string details;
  std::string success;
};

namespace RobotNamespace {

/// 
/// class Robot

class Robot
{
public:
  // Constructors/Destructors  


  /// 
  /// Constructor
  /// Inicializa el robot en un estado seguro y predeterminado.
  Robot();

  /// 
  /// Empty Destructor
  virtual ~Robot();

  void initAttributes();

  /// 
  void connect();

  /// 
  void disconnect();

  /// 
  void enableMotors();

  /// 
  void disableMotors();

  /// 
  /// @param  position 
  /// @param  speed 
  void moveTo(const Position& position, double speed);

  /// @param  position 
  void moveTo(const Position& position);
  void executeMoviment(const Position& position, double speed = 2000.0);

  /// @brief Envía un comando G-Code crudo al robot.
  /// @param gcode El comando G-Code a enviar (ej. "G1 X10").
  void sendRawGCode(const std::string& gcode);

  /// 
  /// @param  active 
  void setEffector(bool active);

  /// @brief Establece el modo de coordenadas del robot.
  /// @param isAbsolute True para modo absoluto (G90), False para modo relativo (G91).
  void setCoordinateMode(bool isAbsolute);

  /// @brief Registra una orden ejecutada en el historial del robot.
  void recordOrder(const std::string& username, const std::string& commandName, 
                   const std::string& details);

  
  void logAndExecuteState(LogLevel level, std::string state);
  void exceptionAndExecute(std::string e);

  void isMoving();

  /// 
  /// @param  command 
  void learnTrajectoryStep(const GCodeNamespace::GCode& command)
  {
  }


  /// 
  /// @param  filePath 
  void executeGCodeFile(std::string filePath)
  {
  }

  /// 
  void parseM114Response(const std::string& response);

  /// 
  /// @return RobotStatus
  RobotStatus getStatus();

private:
  // Private attributes  
  RobotStatus robotStatus;
  std::string connectionStartTime; // New attribute
  std::string executeState;
  std::list<Order> lastOrders; // Lista de las últimas órdenes ejecutadas

public:
  // --- Getters Públicos ---


  /// 
  /// Get the value of connectionStartTime
  /// @return the value of connectionStartTime
  std::string getConnectionStartTime() const {
    return connectionStartTime;
  }
  std::string getExecuteState() const
  {
    return executeState;
  }

  /// 
  /// Get the value of lastOrders
  /// @return the value of lastOrders
  const std::list<Order>& getLastOrders() const
  {
    return lastOrders;
  }



  // --- Setters Públicos ---

  void setExecuteState(const std::string& state)
  {
    executeState = state;
  }
  

  RobotStatus getRobotStatus() const {
    return robotStatus;
  }

  void setRobotStatus(const RobotStatus& status) {
    robotStatus = status;
  }
};

} // namespace RobotNamespace

#endif // ROBOT_H
