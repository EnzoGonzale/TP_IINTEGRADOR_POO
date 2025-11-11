
#ifndef SERIALCOMUNICATOR_H
#define SERIALCOMUNICATOR_H

#include <string>
#include <any>
#include "ISerialCommunicator.h" // Incluimos la nueva interfaz
#include "SerialPortConfiguration.h"


namespace ComunicatorPort{

/// 
/// class SerialCommunicator : public ISerialCommunicator

class SerialComunicator : public ISerialCommunicator
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
  void config(const std::string& port, int speed) override;


  /// 
  /// @return string
  /// @param  message 
  std::string sendMessage(const std::string& message) override;


  /// 
  /// @return string
  /// @param  message 
  std::string reciveMessage(int time = 2) override;


  /// 
  void cleanBuffer() override;


  /// 
  void close() override;

  bool isConfigured() const override {
    return isConfigured_;
  }


  // Prevent copying to avoid double-free issues
  SerialComunicator(const SerialComunicator&) = delete;
  SerialComunicator& operator=(const SerialComunicator&) = delete;
    
  // Allow moving if needed
  SerialComunicator(SerialComunicator&&) noexcept = default;
  SerialComunicator& operator=(SerialComunicator&&) noexcept = default;


private:
  // Private attributes  
  ConfigurationPort::SerialPortConfiguration configurator_;
  int fileDescriptor_ = -1;
  char buffer_[4096];
  bool isConfigured_ = false;

  // Public attribute accessor methods  

  void initAttributes();

};

} // namespace ComunicatorPort
#endif // SERIALCOMUNICATOR_H
