
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include <sys/stat.h>
#include <ctime>


namespace FileNamespace
{

/// 
/// class FileManager

class FileManager
{
public:
  // Constructors/Destructors  



  /// 
  /// Empty Constructor
  FileManager();

  /// 
  /// Empty Destructor
  virtual ~FileManager();



  /// 
  /// @param  fileName 
  /// @param  data 
  /// @param  addMode 
  bool write(std::string fileName, std::string data, bool addMode = false);


  /// 
  /// @return string
  /// @param  fileName 
  std::string read(std::string fileName);


  /// 
  /// @return string
  /// @param  fileName 
  std::string getCreationDate(std::string fileName);

  
  /// 
  /// @return string
  /// @param  fileName 
  std::string getModificationDate(std::string fileName);


private:
  std::string formatDate(time_t time);
  std::string creationDate_;

};

} // namespace FileNamespace

#endif // FILEMANAGER_H
