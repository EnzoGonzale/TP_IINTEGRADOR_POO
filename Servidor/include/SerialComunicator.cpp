#include "SerialComunicator.h"

#include <iostream>
#include <unistd.h>  // For read(), write(), close()
#include <stdexcept> // For std::runtime_error
#include <fcntl.h>   // For file control options
#include <cerrno>    // Para errno
#include <termios.h> // For tcflush()
// Constructors/Destructors


ComunicatorPort::SerialComunicator::SerialComunicator()
{
  initAttributes();
}

ComunicatorPort::SerialComunicator::~SerialComunicator()
{
  close();
}

// Methods

void ComunicatorPort::SerialComunicator::config(std::string port, int speed)
{
  // 1. Abrir el puerto. Esta es la única vez que se llama a open().
  fileDescriptor_ = open(port.c_str(), O_RDWR | O_NOCTTY);
  if (fileDescriptor_ == -1)
  {
      throw std::runtime_error("No se pudo abrir el puerto serie: " + port);
  }

  // 2. Aplicar la configuración al descriptor de archivo que acabamos de abrir.
  configurator_.applyConfig(fileDescriptor_, speed);

  std::cout << "Puerto serie " << port << " abierto con éxito a " << speed << " baudios." << std::endl;
}

std::string ComunicatorPort::SerialComunicator::sendMessage(std::string message)
{
  if (fileDescriptor_ == -1)
  {
      throw std::runtime_error("Puerto serie no configurado.");
  }

  ssize_t bytesWritten = write(fileDescriptor_, message.c_str(), message.size());
  if (bytesWritten == -1)
  {
      throw std::runtime_error("Error al enviar el mensaje.");
  }

  return "Mensaje enviado."; // El método ahora retorna inmediatamente.
}

std::string ComunicatorPort::SerialComunicator::reciveMessage()
{
  if (fileDescriptor_ == -1)
  {
      throw std::runtime_error("No se puede recibir datos, el puerto no está configurado.");
  }

  ssize_t bytesRead = read(fileDescriptor_, buffer_, sizeof(buffer_) - 1);
  if (bytesRead < 0)
  {
      // En modo no bloqueante (O_NDELAY), si read() devuelve -1,
      // debemos comprobar 'errno'. EAGAIN o EWOULDBLOCK significan
      // "no hay datos disponibles ahora", lo cual no es un error fatal.
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
          return ""; // No hay datos, devolvemos una cadena vacía.
      }
      // Si es otro tipo de error, sí es un problema real.
      throw std::runtime_error("Error al leer desde el puerto serie.");
  }

  buffer_[bytesRead] = '\0'; // Null-terminate the received string
  return std::string(buffer_, bytesRead); // Creamos el string con el tamaño exacto leído.
}


void ComunicatorPort::SerialComunicator::cleanBuffer()
{
    if (fileDescriptor_ != -1)
    {
        // Primero, usamos tcflush para intentar una limpieza instantánea de los buffers de E/S.
        // Esto es muy rápido y descarta cualquier dato que ya esté en el buffer del kernel.
        tcflush(fileDescriptor_, TCIFLUSH);

        // Como el mensaje de Arduino puede llegar justo después de abrir el puerto,
        // es mejor añadir una pequeña espera en la lógica de conexión (`Robot::connect`)
        // antes de llamar a esta función si es necesario.
    }
}


void ComunicatorPort::SerialComunicator::close()
{
  if (fileDescriptor_ != -1)
  {
    ::close(fileDescriptor_); // Usamos ::close para evitar ambigüedad con el método de la clase.
    fileDescriptor_ = -1;
  }
}
// Accessor methods



// Other methods


void ComunicatorPort::SerialComunicator::initAttributes()
{
}
