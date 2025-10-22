#include "SerialPortConfiguration.h"

#include <iostream>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>


// Constructors/Destructors


ConfigurationPort::SerialPortConfiguration::SerialPortConfiguration()
{
}

ConfigurationPort::SerialPortConfiguration::~SerialPortConfiguration()
{
}

// Methods

bool ConfigurationPort::SerialPortConfiguration::applyConfig(int fileDescriptor, int speed)
{
    speed_ = speed;

    // Configurar los atributos del puerto serie
    struct termios tty;
    if (tcgetattr(fileDescriptor, &tty) != 0)
    {
        throw std::runtime_error("Error al obtener los atributos del puerto serie.");
    }

    // Configurar la velocidad
    speed_t baudRate;
    switch (speed_)
    {
    case 300:
        baudRate = B300;
        break;
    case 1200:
        baudRate = B1200;
        break;
    case 2400:
        baudRate = B2400;
        break;
    case 4800:
        baudRate = B4800;
        break;
    case 9600:
        baudRate = B9600;
        break;
    case 19200:
        baudRate = B19200;
        break;
    case 38400:
        baudRate = B38400;
        break;
    case 57600:
        baudRate = B57600;
        break;
    case 115200:
        baudRate = B115200;
        break;
    default:
        throw std::runtime_error("Velocidad no soportada: " + std::to_string(speed_));
    }

    cfsetospeed(&tty, baudRate);
    cfsetispeed(&tty, baudRate);

    // Configurar otros atributos (puedes ajustarlos según tus necesidades)
    tty.c_cflag |= (CLOCAL | CREAD); // No modem control, enable reading
    tty.c_cflag &= ~PARENB;          // No parity
    tty.c_cflag &= ~CSTOPB;          // 1 stop bit
    tty.c_cflag &= ~CSIZE;           // Clear size bits
    tty.c_cflag |= CS8;              // 8 data bits
    tty.c_lflag = 0;                 // No local flags
    tty.c_oflag = 0;                 // No output processing
    tty.c_cc[VMIN] = 1;              // Esperar hasta que haya al menos 1 byte para leer.
    tty.c_cc[VTIME] = 0;             // Sin timeout de lectura. La llamada a read() será bloqueante indefinidamente.

    if (tcsetattr(fileDescriptor, TCSANOW, &tty) != 0)
    {
        throw std::runtime_error("Error al establecer los atributos del puerto serie.");
    }

    return true;
}



// Other methods
