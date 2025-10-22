
#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <string>
#include <map>
#include "User.h"



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
  /// @param  user 
  std::string generateUserReport(UserNamespace::User user)
  {
    return std::string();
  }


  /// 
  /// @return string
  /// @param  filters 
  std::string generateAdminReport(const std::map<std::string, std::string>& filters)
  {
    return std::string();
  }


  /// 
  /// @return string
  /// @param  filters 
  std::string generateLogReport(const std::map<std::string, std::string>& filters)
  {
    return std::string();
  }


};

#endif // REPORTGENERATOR_H
