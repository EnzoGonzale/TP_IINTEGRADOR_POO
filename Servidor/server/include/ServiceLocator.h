#ifndef SERVICELOCATOR_H
#define SERVICELOCATOR_H

#include "ISerialCommunicator.h"
#include <memory>
#include <stdexcept>

/// @brief Proporciona acceso global a servicios como el comunicador serie.
class ServiceLocator {
public:
    /// @brief Obtiene una referencia al comunicador serie registrado.
    static ComunicatorPort::ISerialCommunicator& getCommunicator() {
        if (!communicator_) {
            throw std::runtime_error("ServiceLocator: No ISerialCommunicator provided.");
        }
        return *communicator_;
    }

    /// @brief Registra una implementación de ISerialCommunicator.
    static void provide(ComunicatorPort::ISerialCommunicator* communicator) {
        communicator_ = communicator;
    }

private:
    inline static ComunicatorPort::ISerialCommunicator* communicator_ = nullptr;
};

#endif // SERVICELOCATOR_H