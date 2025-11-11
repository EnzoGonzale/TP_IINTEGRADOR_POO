
#ifndef SERIALPORTCONFIGURATION_H
#define SERIALPORTCONFIGURATION_H
#include <any>
#include <string>


namespace ConfigurationPort{

/// 
/// class SerialPortConfiguration

class SerialPortConfiguration
{
public:
  // Constructors/Destructors  



  /// 
  /// Empty Constructor
  SerialPortConfiguration();

  /// 
  /// Empty Destructor
  virtual ~SerialPortConfiguration();



  /// 
  /// @param  port 
  /// @param  speed 
  /// @param  fileDescriptor El descriptor del archivo del puerto a configurar.
  bool applyConfig(int fileDescriptor, int speed);

private:

  std::string port_;
  int speed_;

};

} // namespace ConfigurationPort
#endif // SERIALPORTCONFIGURATION_H
