
#ifndef SERIALCOMUNICATOR_H
#define SERIALCOMUNICATOR_H

#include <string>
#include <any>
#include "SerialPortConfiguration.h"


namespace ComunicatorPort{

/// 
/// class SerialComunicator

class SerialComunicator
{
public:
  // Constructors/Destructors  



  /// 
  /// Empty Constructor
  SerialComunicator();

  /// 
  /// Empty Destructor
  virtual ~SerialComunicator();



  /// 
  /// @param  port 
  /// @param  speed 
  void config(std::string port, int speed);


  /// 
  /// @return string
  /// @param  message 
  std::string sendMessage(std::string message);


  /// 
  /// @return string
  /// @param  message 
  std::string reciveMessage();


  /// 
  void cleanBuffer();


  /// 
  void close();

private:
  // Private attributes  
  ConfigurationPort::SerialPortConfiguration configurator_;
  int fileDescriptor_ = -1;
  char buffer_[4096];

  // Public attribute accessor methods  

  void initAttributes();

};

} // namespace ComunicatorPort
#endif // SERIALCOMUNICATOR_H
