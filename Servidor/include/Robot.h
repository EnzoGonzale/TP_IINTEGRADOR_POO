
#ifndef ROBOT_H
#define ROBOT_H

#include <string>
#include <any>
#include <list>
#include "Position.h"
#include "GCode.h"
#include "SerialComunicator.h"

// --- Definiciones de Platzhalter ---
// Estas definiciones deben moverse a sus propios archivos .h a medida que las desarrolles.
struct RobotStatus {
  bool isConnected;
  bool areMotorsEnabled;
  // ... otros campos de estado
};

struct Order {
  std::string command;
  // ... otros detalles de la orden
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

  /// 
  /// @param  active 
  void setEffector(bool active);

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
  /// @return RobotStatus
  RobotStatus getStatus() const
  {
    RobotStatus status;
    status.isConnected = isConnected;
    status.areMotorsEnabled = areMotorsEnabled;
    // ... llenar otros campos de estado
    return status;
  }

private:
  // Private attributes  


  bool isConnected;
  bool areMotorsEnabled;
  Position currentPosition;
  std::string activityState;
  std::list<Order> lastOrders; // Lista de las últimas órdenes ejecutadas
  ComunicatorPort::SerialComunicator serialCommunicator;

public:
  // --- Getters Públicos ---

  /// 
  /// Get the value of isConnected
  /// @return the value of isConnected
  bool getIsConnected() const
  {
    return isConnected;
  }

  /// 
  /// Get the value of areMotorsEnabled
  /// @return the value of areMotorsEnabled
  bool getAreMotorsEnabled() const
  {
    return areMotorsEnabled;
  }

  /// 
  /// Get the value of currentPosition
  /// @return the value of currentPosition
  const Position& getCurrentPosition() const
  {
    return currentPosition;
  }

  /// 
  /// Get the value of activityState
  /// @return the value of activityState
  std::string getActivityState() const
  {
    return activityState;
  }

  /// 
  /// Get the value of lastOrders
  /// @return the value of lastOrders
  const std::list<Order>& getLastOrders() const
  {
    return lastOrders;
  }

};

} // namespace RobotNamespace

#endif // ROBOT_H
