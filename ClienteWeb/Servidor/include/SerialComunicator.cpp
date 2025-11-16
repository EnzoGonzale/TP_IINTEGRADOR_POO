#include "SerialComunicator.h"

#include <iostream>
#include <unistd.h>  // For read(), write(), close()
#include <stdexcept> // For std::runtime_error
#include <fcntl.h>   // For file control options
#include <chrono>    // Para el manejo del tiempo
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

   isConfigured_ = true;
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

std::string ComunicatorPort::SerialComunicator::reciveMessage(int time)
{
  if (fileDescriptor_ == -1)
  {
      throw std::runtime_error("No se puede recibir datos, el puerto no está configurado.");
  }

    std::string full_response;
    auto startTime = std::chrono::steady_clock::now();
    
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - startTime).count() < time) {
        ssize_t bytesRead = read(fileDescriptor_, buffer_, sizeof(buffer_) - 1);
        
        if (bytesRead > 0) {
            // Si leemos datos, los añadimos a nuestra respuesta.
            full_response.append(buffer_, bytesRead);
        } else if (bytesRead < 0) {
            // En modo no bloqueante, EAGAIN o EWOULDBLOCK no son errores,
            // simplemente significan que no hay datos ahora.
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                throw std::runtime_error("Error al leer desde el puerto serie.");
            }
        }
        // Si bytesRead es 0 o -1 (con EAGAIN), simplemente continuamos el bucle
        // hasta que se cumpla el tiempo.
    }

    return full_response;
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
