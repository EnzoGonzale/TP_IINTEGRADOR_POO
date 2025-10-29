
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include <sys/stat.h>
#include <ctime>
#include <fstream> // Para std::ofstream


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

  // --- Nuevos métodos para gestionar un archivo abierto ---

  /// @brief Abre un archivo y lo mantiene abierto para futuras escrituras.
  /// @param fileName La ruta al archivo.
  /// @param addMode True para abrir en modo de añadir (append), false para sobrescribir.
  /// @return True si el archivo se abrió con éxito, false en caso contrario.
  bool open(const std::string& fileName, bool addMode = true);

  /// @brief Cierra el archivo que se mantiene abierto.
  void close();

  /// @brief Escribe datos en el archivo abierto.
  /// @param data La cadena de texto a escribir.
  void write(const std::string& data);

  /// @brief Vuelca el buffer del archivo al disco.
  void flush();

  /// @brief Comprueba si el archivo está actualmente abierto.
  bool isOpen() const;

  /// 
  /// @param  fileName 
  /// @param  data 
  /// @param  addMode 
  bool write(std::string fileName, std::string data, bool addMode = false);


  /// 
  /// @return string
  /// @param  fileName 
  std::string read(std::string fileName);


  // /// 
  // /// @return string
  // /// @param  fileName 
  // std::string getCreationDate(std::string fileName);

  
  // /// 
  // /// @return string
  // /// @param  fileName 
  // std::string getModificationDate(std::string fileName);


private:
  // std::string formatDate(time_t time);
  // std::string creationDate_;
  std::ofstream fileStream_; // Stream para mantener el archivo abierto

};

} // namespace FileNamespace

#endif // FILEMANAGER_H
