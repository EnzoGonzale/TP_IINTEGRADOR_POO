#include "RpcServiceHandler.h"
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include "Logger.h" // Incluimos el Logger
#include <iostream> // Para std::cout

// Constructors/Destructors


RpcServiceHandlerNamespace::RpcServiceHandler::RpcServiceHandler(
    AuthenticationServiceNamespace::AuthenticationService& authService,
    RobotNamespace::Robot& robot
) : authService(authService), robot(robot)
{
}

RpcServiceHandlerNamespace::RpcServiceHandler::~RpcServiceHandler()
{
}
 
// --- Clase Base para Métodos RPC con Autenticación ---
// Centraliza la lógica de autenticación para no repetirla en cada método.
class AuthenticatedMethod : public xmlrpc_c::method {
protected:
    AuthenticationServiceNamespace::AuthenticationService& authService;
    RobotNamespace::Robot& robot;
    std::string _name; // Añadimos el miembro _name que faltaba.

    AuthenticatedMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r)
        : authService(auth), robot(r) {}

    // El método execute ahora es un "Template Method". Realiza la autenticación
    // y luego delega la ejecución real al método 'executeAuthenticated'.
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP) override {
        try {
            std::string const username(paramList.getString(0));
            std::string const password(paramList.getString(1));
            paramList.verifyEnd(2); // Por defecto, los dos primeros son user/pass

            auto userOpt = authService.authenticate(username, password);
            if (!userOpt) {
                throw xmlrpc_c::fault("Authentication failed: Invalid username or password.", xmlrpc_c::fault::CODE_INTERNAL);
            }

            // Log de la llamada
            std::string logMessage = "RPC call from user '" + username + "': " + this->_name;
            std::cout << "[RPC] " << logMessage << std::endl;
            Logger::getInstance().log(LogLevel::INFO, logMessage);

            // Llama a la lógica específica del método hijo.
            executeAuthenticated(paramList, retvalP, *userOpt);

        } catch (std::exception const& e) {
            // Captura errores de autenticación o de ejecución y los devuelve como un "fault" RPC.
            throw xmlrpc_c::fault(e.what(), xmlrpc_c::fault::CODE_INTERNAL);
        }
    }

    // Método virtual puro que las clases hijas deben implementar con su lógica específica.
    virtual void executeAuthenticated(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP, UserNamespace::User& user) = 0;
};

// --- Implementación de los Métodos RPC Reales ---

class RobotConnectMethod : public AuthenticatedMethod {
public:
    RobotConnectMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r)
        : AuthenticatedMethod(auth, r) {
        this->_signature = "b:ss"; // boolean robot.connect(string user, string pass)
        this->_name = "robot.connect";
        this->_help = "Connects to the robot after authenticating the user.";
    }
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP, UserNamespace::User& user) override {
        robot.connect();
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};

class RobotDisconnectMethod : public AuthenticatedMethod {
public:
    RobotDisconnectMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r)
        : AuthenticatedMethod(auth, r) {
        this->_signature = "b:ss"; // boolean robot.disconnect(string user, string pass)
        this->_name = "robot.disconnect";
        this->_help = "Disconnects from the robot after authenticating the user.";
    }
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP, UserNamespace::User& user) override {
        robot.disconnect();
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};

class RobotGetStatusMethod : public AuthenticatedMethod {
public:
    RobotGetStatusMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r)
        : AuthenticatedMethod(auth, r) {
        this->_signature = "S:ss"; // struct robot.getStatus(string user, string pass)
        this->_name = "robot.getStatus";
        this->_help = "Gets the robot's current status after authenticating the user.";
    }
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP, UserNamespace::User& user) override {
        RobotStatus status = robot.getStatus();
        Position pos = robot.getCurrentPosition();

        std::map<std::string, xmlrpc_c::value> statusMap;
        statusMap["isConnected"] = xmlrpc_c::value_boolean(status.isConnected);
        statusMap["areMotorsEnabled"] = xmlrpc_c::value_boolean(status.areMotorsEnabled);
        statusMap["activityState"] = xmlrpc_c::value_string(robot.getActivityState());
        
        std::map<std::string, xmlrpc_c::value> positionMap;
        positionMap["x"] = xmlrpc_c::value_double(pos.x);
        positionMap["y"] = xmlrpc_c::value_double(pos.y);
        positionMap["z"] = xmlrpc_c::value_double(pos.z);
        statusMap["position"] = xmlrpc_c::value_struct(positionMap);

        *retvalP = xmlrpc_c::value_struct(statusMap);
    }
};

class RobotMoveMethod : public AuthenticatedMethod {
public:
    RobotMoveMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r)
        : AuthenticatedMethod(auth, r) {
        this->_signature = "b:ssdddd"; // boolean robot.move(string user, string pass, double x, double y, double z, double speed)
        this->_name = "robot.move";
        this->_help = "Moves the robot to a specific position.";
    }

    // Sobreescribimos 'execute' para manejar los parámetros adicionales (x, y, z, speed)
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP) override {
        try {
            std::string const username(paramList.getString(0));
            std::string const password(paramList.getString(1));
            double const x(paramList.getDouble(2));
            double const y(paramList.getDouble(3));
            double const z(paramList.getDouble(4));
            double const speed(paramList.getDouble(5));
            paramList.verifyEnd(6);

            auto userOpt = authService.authenticate(username, password);
            if (!userOpt) {
                throw xmlrpc_c::fault("Authentication failed: Invalid username or password.", xmlrpc_c::fault::CODE_INTERNAL);
            }

            std::string logMessage = "RPC call from user '" + username + "': " + this->_name;
            std::cout << "[RPC] " << logMessage << std::endl;
            Logger::getInstance().log(LogLevel::INFO, logMessage);

            // Lógica específica del método
            robot.moveTo(Position(x, y, z), speed);
            *retvalP = xmlrpc_c::value_boolean(true);

        } catch (std::exception const& e) {
            throw xmlrpc_c::fault(e.what(), xmlrpc_c::fault::CODE_INTERNAL);
        }
    }

    // Este método no se usará porque sobreescribimos 'execute' directamente, pero debe estar definido.
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP, UserNamespace::User& user) override {
        // No se llega aquí.
    }
};
 
class RobotEnableMotorsMethod : public AuthenticatedMethod {
public:
    RobotEnableMotorsMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r)
        : AuthenticatedMethod(auth, r) {
        this->_signature = "b:ss"; // boolean robot.enableMotors(string user, string pass)
        this->_name = "robot.enableMotors";
        this->_help = "Enables the robot motors.";
    }
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP, UserNamespace::User& user) override {
        robot.enableMotors();
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};

class RobotDisableMotorsMethod : public AuthenticatedMethod {
public:
    RobotDisableMotorsMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r)
        : AuthenticatedMethod(auth, r) {
        this->_signature = "b:ss"; // boolean robot.disableMotors(string user, string pass)
        this->_name = "robot.disableMotors";
        this->_help = "Disables the robot motors.";
    }
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP, UserNamespace::User& user) override {
        robot.disableMotors();
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};

class RobotSetEffectorMethod : public AuthenticatedMethod {
public:
    RobotSetEffectorMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r)
        : AuthenticatedMethod(auth, r) {
        this->_signature = "b:ssb"; // boolean robot.setEffector(string user, string pass, boolean active)
        this->_name = "robot.setEffector";
        this->_help = "Sets the state of the robot's end effector.";
    }

    // Sobrescribimos 'execute' para manejar el parámetro booleano adicional.
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP) override {
        try {
            // 1. Extraemos todos los parámetros primero.
            std::string const username(paramList.getString(0));
            std::string const password(paramList.getString(1));
            bool const active(paramList.getBoolean(2));
            paramList.verifyEnd(3); // Verificamos que hay exactamente 3 parámetros.

            // 2. Realizamos la autenticación.
            auto userOpt = authService.authenticate(username, password);
            if (!userOpt) {
                throw xmlrpc_c::fault("Authentication failed: Invalid username or password.", xmlrpc_c::fault::CODE_INTERNAL);
            }

            // 3. Ejecutamos la lógica específica del método.
            robot.setEffector(active);
            *retvalP = xmlrpc_c::value_boolean(true);

        } catch (std::exception const& e) {
            throw xmlrpc_c::fault(e.what(), xmlrpc_c::fault::CODE_INTERNAL);
        }
    }

    void executeAuthenticated(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP, UserNamespace::User& user) override {
        // Este método ya no se usará porque hemos sobrescrito 'execute' directamente.
    }
};

// --- Método especial para la autenticación ---
// No hereda de AuthenticatedMethod porque este es el punto de entrada.
class RobotAuthenticateMethod : public xmlrpc_c::method {
private:
    std::string _name;
    AuthenticationServiceNamespace::AuthenticationService& authService;
public:
    RobotAuthenticateMethod(AuthenticationServiceNamespace::AuthenticationService& auth)
        : authService(auth) {
        this->_signature = "S:ss"; // struct robot.authenticate(string user, string pass)
        this->_name = "robot.authenticate";
        this->_help = "Authenticates a user and returns {authenticated:bool, role:int}.";
    }

    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP) override {
        std::string const username(paramList.getString(0));
        std::string const password(paramList.getString(1));
        paramList.verifyEnd(2);

        auto userOpt = authService.authenticate(username, password);
        
        std::map<std::string, xmlrpc_c::value> result_map;
        if (userOpt) {
            result_map["authenticated"] = xmlrpc_c::value_boolean(true);
            result_map["role"] = xmlrpc_c::value_int(static_cast<int>(userOpt->getRole()));
        } else {
            result_map["authenticated"] = xmlrpc_c::value_boolean(false);
            result_map["role"] = xmlrpc_c::value_int(-1); // Rol inválido para indicar fallo
        }
        *retvalP = xmlrpc_c::value_struct(result_map);
    }
};

// --- Método para añadir usuarios (solo para administradores) ---
class RobotUserAddMethod : public AuthenticatedMethod {
public:
    RobotUserAddMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r)
        : AuthenticatedMethod(auth, r) {
        // Firma: bool user_add(string adminUser, string adminPass, string newUser, string newPass, int newRole)
        this->_signature = "b:ssssi";
        this->_name = "robot.user_add";
        this->_help = "Adds a new user. Requires administrator privileges.";
    }

    // Sobrescribimos 'execute' para manejar los parámetros adicionales.
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP) override {
        try {
            // 1. Extraemos las credenciales del administrador para autenticar.
            std::string const adminUsername(paramList.getString(0));
            std::string const adminPassword(paramList.getString(1));

            // 2. Realizamos la autenticación.
            auto userOpt = authService.authenticate(adminUsername, adminPassword);
            if (!userOpt) {
                throw xmlrpc_c::fault("Authentication failed: Invalid admin username or password.", xmlrpc_c::fault::CODE_INTERNAL);
            }

            // 3. Si la autenticación es exitosa, llamamos a la lógica específica del método.
            // Pasamos la lista de parámetros completa y el objeto de usuario autenticado.
            executeAuthenticated(paramList, retvalP, *userOpt);

        } catch (std::exception const& e) {
            // Captura errores de autenticación o de ejecución y los devuelve como un "fault" RPC.
            throw xmlrpc_c::fault(e.what(), xmlrpc_c::fault::CODE_INTERNAL);
        }
    }


    void executeAuthenticated(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP, UserNamespace::User& adminUser) override {
        // 1. Verificar que el usuario autenticado es un administrador.
        if (adminUser.getRole() != UserRole::ADMIN) {
            throw xmlrpc_c::fault("Permission denied: Only administrators can add users.", xmlrpc_c::fault::CODE_INTERNAL);
        }

        // 2. Extraer los parámetros del nuevo usuario.
        // Los parámetros del admin (0 y 1) ya fueron usados para la autenticación.
        std::string const newUsername(paramList.getString(2));
        std::string const newPassword(paramList.getString(3));
        int         const newRoleInt(paramList.getInt(4));
        paramList.verifyEnd(5);

        UserRole newRole = static_cast<UserRole>(newRoleInt);

        // 3. Llamar al servicio para crear el usuario.
        bool success = authService.createUser(newUsername, newPassword, newRole);
        if (!success) {
            throw xmlrpc_c::fault("Failed to create user. It might already exist.", xmlrpc_c::fault::CODE_INTERNAL);
        }

        *retvalP = xmlrpc_c::value_boolean(success);
    }
};

void RpcServiceHandlerNamespace::RpcServiceHandler::registerMethods(xmlrpc_c::registry &registry) {
    // Registramos el método de autenticación.
    registry.addMethod("robot.authenticate", new RobotAuthenticateMethod(authService));
    // Registramos los demás métodos protegidos.
    registry.addMethod("robot.connect", new RobotConnectMethod(authService, robot));
    registry.addMethod("robot.disconnect", new RobotDisconnectMethod(authService, robot));
    registry.addMethod("robot.user_add", new RobotUserAddMethod(authService, robot));
    registry.addMethod("robot.getStatus", new RobotGetStatusMethod(authService, robot));
    registry.addMethod("robot.move", new RobotMoveMethod(authService, robot));
    registry.addMethod("robot.enableMotors", new RobotEnableMotorsMethod(authService, robot));
    registry.addMethod("robot.disableMotors", new RobotDisableMotorsMethod(authService, robot));
    registry.addMethod("robot.setEffector", new RobotSetEffectorMethod(authService, robot));
}
