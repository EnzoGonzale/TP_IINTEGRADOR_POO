#ifndef ISERIALCOMMUNICATOR_H
#define ISERIALCOMMUNICATOR_H

#include <string>

namespace ComunicatorPort {

class ISerialCommunicator {
public:
    virtual ~ISerialCommunicator() = default;

    virtual void config(const std::string& port, int speed) = 0;
    virtual std::string sendMessage(const std::string& message) = 0;
    virtual std::string reciveMessage(int time = 2) = 0;
    virtual void cleanBuffer() = 0;
    virtual void close() = 0;
    virtual bool isConfigured() const = 0;
};

} // namespace ComunicatorPort

#endif // ISERIALCOMMUNICATOR_H