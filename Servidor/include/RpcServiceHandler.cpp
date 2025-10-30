extern "C" {
    // Incluimos abyss.h explícitamente para tener acceso a las funciones de C de Abyss.
    // DEBE estar antes que cualquier otro include de xmlrpc-c para que funcione.
    #include <xmlrpc-c/abyss.h>
    #include <arpa/inet.h> // Para inet_ntoa
}
#include <iomanip> // Para std::setprecision
#include <thread>   // Para std::this_thread
#include <chrono>   // Para std::chrono
#include "RpcServiceHandler.h"
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/server_abyss.hpp> // Necesario para callInfo_serverAbyss
// #include "Logger.h" // Incluimos el Logger
#include "TaskManager.h"
#include "Utils.h" // Incluimos nuestro nuevo archivo de utilidades
#include "Exceptions.h"

// Constructors/Destructors

RpcServiceHandlerNamespace::RpcServiceHandler::RpcServiceHandler(
    AuthenticationServiceNamespace::AuthenticationService& authService,
    RobotNamespace::Robot& robot,
    TaskManager& taskManager
) : authService(authService), robot(robot), taskManager(taskManager)
{
}

RpcServiceHandlerNamespace::RpcServiceHandler::~RpcServiceHandler()
{
}
 
// --- Clase Base para Métodos RPC con Autenticación ---
// Centraliza la lógica de autenticación para no repetirla en cada método.
class AuthenticatedMethod : public xmlrpc_c::method2 {
protected:
    AuthenticationServiceNamespace::AuthenticationService& authService;
    RobotNamespace::Robot& robot;
    TaskManager& taskManager;
    std::string _name; // Añadimos el miembro _name que faltaba.

    AuthenticatedMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : authService(auth), robot(r), taskManager(tm) {}

    // El método execute ahora es un "Template Method". Realiza la autenticación
    // y luego delega la ejecución real al método 'executeAuthenticated'.
    // La firma de execute incluye un puntero a callInfo, que contiene datos de la conexión.
    void execute(xmlrpc_c::paramList const& paramList,
                 const xmlrpc_c::callInfo * const callInfoP,
                 xmlrpc_c::value*       const retvalP) override {
        
            std::string const username(paramList.getString(0));
            std::string const password(paramList.getString(1));
           
            // Obtenemos la IP del cliente de forma segura.
            std::string clientIp = "unknown";
            auto const * const abyssCallInfoP =
                dynamic_cast<const xmlrpc_c::callInfo_serverAbyss*>(callInfoP);
            
            if (abyssCallInfoP) {
                // Usamos SessionGetChannelInfo para obtener la IP, como lo investigaste.
                struct abyss_unix_chaninfo *channelInfo;
                SessionGetChannelInfo(abyssCallInfoP->abyssSessionP, (void**)&channelInfo);
                if (channelInfo)
                    clientIp = inet_ntoa(((struct sockaddr_in*)&channelInfo->peerAddr)->sin_addr);
            }
            
        try { 
            auto userOpt = authService.authenticate(username, password); // Ahora puede lanzar excepciones
            // Log de la llamada
            std::string logMessage = "RPC call from user '" + username + "': " + this->_name;
            Logger::getInstance().log(LogLevel::INFO, "[RPC] " + logMessage, username, clientIp);

            // Llama a la lógica específica del método hijo.
            // Los métodos que no necesitan parámetros adicionales ignoran el paramList.
            // Por ejemplo, connect, disconnect, getStatus, etc.
            executeAuthenticated(paramList, retvalP, *userOpt, clientIp);

        } catch (const AuthenticationException& e) {
            // Capturamos específicamente las excepciones de autenticación
            Logger::getInstance().log(LogLevel::WARNING, "Failed RPC call: " + std::string(e.what()), username, clientIp);
            throw xmlrpc_c::fault(e.what(), xmlrpc_c::fault::CODE_INTERNAL);
        } catch (const std::exception& e) {
            // Captura errores de autenticación o de ejecución y los devuelve como un "fault" RPC.
            robot.recordOrder(username, "ERROR", e.what());
            throw xmlrpc_c::fault(e.what(), xmlrpc_c::fault::CODE_INTERNAL);
        }
    }

    // Método virtual puro que las clases hijas deben implementar con su lógica específica.
    virtual void executeAuthenticated(xmlrpc_c::paramList const& paramList, 
                                      xmlrpc_c::value* const retvalP, 
                                      UserNamespace::User& user,
                                      const std::string& clientIp) = 0;
};

// --- Implementación de los Métodos RPC Reales ---

class RobotConnectMethod : public AuthenticatedMethod {
public:
    RobotConnectMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "b:ss"; // boolean robot.connect(string user, string pass)
        this->_name = "robot.connect";
        this->_help = "Connects to the robot after authenticating the user.";
    }
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, 
                              xmlrpc_c::value* const retvalP, 
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        robot.connect();
        paramList.verifyEnd(2);
        robot.recordOrder(user.getUsername(), "connect", "CONECTADO");
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};

class RobotDisconnectMethod : public AuthenticatedMethod {
public:
    RobotDisconnectMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "b:ss"; // boolean robot.disconnect(string user, string pass)
        this->_name = "robot.disconnect";
        this->_help = "Disconnects from the robot after authenticating the user.";
    }
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, 
                              xmlrpc_c::value* const retvalP, 
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        robot.disconnect();
        paramList.verifyEnd(2);
        robot.recordOrder(user.getUsername(), "disconnect", "DESCONECTADO");
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};

class RobotGetStatusMethod : public AuthenticatedMethod {
public:
    RobotGetStatusMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "S:ss"; // struct robot.getStatus(string user, string pass)
        this->_name = "robot.getStatus";
        this->_help = "Gets the robot's current status after authenticating the user.";
    }
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, 
                              xmlrpc_c::value* const retvalP, 
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        RobotStatus status = robot.getStatus();
        paramList.verifyEnd(2);

        std::map<std::string, xmlrpc_c::value> statusMap;
        statusMap["isConnected"] = xmlrpc_c::value_boolean(status.isConnected);
        statusMap["areMotorsEnabled"] = xmlrpc_c::value_boolean(status.areMotorsEnabled);
        statusMap["activityState"] = xmlrpc_c::value_string(status.activityState);
        statusMap["coordinateMode"] = xmlrpc_c::value_string(status.isAbsolute ? "ABSOLUTO" : "RELATIVO");
        
        std::map<std::string, xmlrpc_c::value> positionMap;
        positionMap["x"] = xmlrpc_c::value_double(status.currentPosition.x);
        positionMap["y"] = xmlrpc_c::value_double(status.currentPosition.y);
        positionMap["z"] = xmlrpc_c::value_double(status.currentPosition.z);
        statusMap["position"] = xmlrpc_c::value_struct(positionMap);

        *retvalP = xmlrpc_c::value_struct(statusMap);
    }
};

// --- Método para mover el robot ---
class RobotMoveMethod : public AuthenticatedMethod {
public:
    RobotMoveMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "b:ssdddd"; // boolean robot.move(string user, string pass, double x, double y, double z, double speed)
        this->_name = "robot.move";
        this->_help = "Moves the robot to a specific position.";
    }

    // Implementamos el método virtual puro, que ahora recibe la IP.
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, 
                              xmlrpc_c::value* const retvalP, 
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        // Los parámetros 0 y 1 (user, pass) ya fueron consumidos por la clase base.
        // Aquí leemos los parámetros específicos de este método.
        double const x(paramList.getDouble(2));
        double const y(paramList.getDouble(3));
        double const z(paramList.getDouble(4));
        double const speed(paramList.getDouble(5));
        paramList.verifyEnd(6);

        // Lógica específica del método
        robot.moveTo(Position(x, y, z), speed);
        robot.recordOrder(user.getUsername(), "move", 
                          "Moved to (" + double_a_string_con_precision(x, 2)+ ", " + 
                          double_a_string_con_precision(y, 2)+ ", " + 
                          double_a_string_con_precision(z, 2)+ ") at speed " + 
                          double_a_string_con_precision(speed, 2));
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};
 

// --- Método para mover el robot con velocidad por defecto ---
class RobotMoveDefaultSpeedMethod : public AuthenticatedMethod {
public:
    RobotMoveDefaultSpeedMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "b:ssddd"; // boolean robot.moveDefaultSpeed(string user, string pass, double x, double y, double z)
        this->_name = "robot.moveDefaultSpeed";
        this->_help = "Moves the robot to a specific position using default speed.";
    }

    void executeAuthenticated(xmlrpc_c::paramList const& paramList, 
                              xmlrpc_c::value* const retvalP, 
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        double const x(paramList.getDouble(2));
        double const y(paramList.getDouble(3));
        double const z(paramList.getDouble(4));
        paramList.verifyEnd(5);

        // Llama a la sobrecarga de moveTo que usa la velocidad por defecto
        robot.moveTo(Position(x, y, z)); 
        robot.recordOrder(user.getUsername(), "move", 
                          "Moved to (" + double_a_string_con_precision(x, 2)+ ", " + 
                          double_a_string_con_precision(y, 2)+ ", " + 
                          double_a_string_con_precision(z, 2)+ ") at default speed");
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};


// --- Método para activar los motores ---
class RobotEnableMotorsMethod : public AuthenticatedMethod {
public:
    RobotEnableMotorsMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "b:ss"; // boolean robot.enableMotors(string user, string pass)
        this->_name = "robot.enableMotors";
        this->_help = "Enables the robot motors.";
    }
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, 
                              xmlrpc_c::value* const retvalP, 
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        robot.enableMotors();
        paramList.verifyEnd(2);
        robot.recordOrder(user.getUsername(), "enableMotors", "MOTORES ACTIVADOS");
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};

// --- Método para desactivar los motores ---
class RobotDisableMotorsMethod : public AuthenticatedMethod {
public:
    RobotDisableMotorsMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "b:ss"; // boolean robot.disableMotors(string user, string pass)
        this->_name = "robot.disableMotors";
        this->_help = "Disables the robot motors.";
    }
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, 
                              xmlrpc_c::value* const retvalP, 
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        robot.disableMotors();
        paramList.verifyEnd(2);
        robot.recordOrder(user.getUsername(), "disableMotors", "MOTORES DESACTIVADOS");
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};

// --- Método para activar/desactivar el efector final ---
class RobotSetEffectorMethod : public AuthenticatedMethod {
public:
    RobotSetEffectorMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "b:ssb"; // boolean robot.setEffector(string user, string pass, boolean active)
        this->_name = "robot.setEffector";
        this->_help = "Sets the state of the robot's end effector.";
    }

    // Implementamos el método virtual puro.
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, 
                              xmlrpc_c::value* const retvalP, 
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        // Leemos el parámetro booleano específico de este método.
        bool const active(paramList.getBoolean(2));
        paramList.verifyEnd(3);

        // Lógica específica del método.
        robot.setEffector(active);
        std::string details = active ? "ACTIVO" : "INACTIVO";
        robot.recordOrder(user.getUsername(), "setEffector", details);
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};

// --- Método para cambiar el modo de coordenadas (Absoluto/Relativo) ---
class SetCoordinateModeMethod : public AuthenticatedMethod {
public:
    SetCoordinateModeMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "b:ssb"; // boolean robot.setCoordinateMode(string user, string pass, boolean isAbsolute)
        this->_name = "robot.setCoordinateMode";
        this->_help = "Sets the robot's coordinate mode (G90 for absolute, G91 for relative).";
    }

    void executeAuthenticated(xmlrpc_c::paramList const& paramList,
                              xmlrpc_c::value* const retvalP,
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        // Leemos el parámetro booleano específico de este método.
        bool const isAbsolute(paramList.getBoolean(2));
        paramList.verifyEnd(3);

        // Llama a un método en Robot que envíe el comando G-Code apropiado.
        robot.setCoordinateMode(isAbsolute);
        std::string details = isAbsolute ? "ABSOLUTO (G90)" : "RELATIVO (G91)";
        robot.recordOrder(user.getUsername(), "setCoordinateMode", details);
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};

// --- Método especial para la autenticación ---
// No hereda de AuthenticatedMethod porque este es el punto de entrada.
class RobotAuthenticateMethod : public xmlrpc_c::method2 {
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

    void execute(xmlrpc_c::paramList const& paramList,
                 const xmlrpc_c::callInfo * const callInfoP,
                 xmlrpc_c::value*       const retvalP) override {
        std::string const username(paramList.getString(0));
        std::string const password(paramList.getString(1));
        paramList.verifyEnd(2);

        // También podemos loguear el intento de login
        std::string clientIp = "unknown";
        auto const * const abyssCallInfoP =
            dynamic_cast<const xmlrpc_c::callInfo_serverAbyss*>(callInfoP);
        
        if (abyssCallInfoP) {
            // Usamos SessionGetChannelInfo para obtener la IP.
            struct abyss_unix_chaninfo *channelInfo;
            SessionGetChannelInfo(abyssCallInfoP->abyssSessionP, (void**)&channelInfo);
            if (channelInfo)
                clientIp = inet_ntoa(((struct sockaddr_in*)&channelInfo->peerAddr)->sin_addr);
        }

        auto userOpt = authService.authenticate(username, password);
        
        std::map<std::string, xmlrpc_c::value> result_map;
        if (userOpt) {
            result_map["authenticated"] = xmlrpc_c::value_boolean(true);
            result_map["role"] = xmlrpc_c::value_int(static_cast<int>(userOpt->getRole()));
            Logger::getInstance().log(LogLevel::INFO, "Successful login attempt", username, clientIp);
        } else {
            result_map["authenticated"] = xmlrpc_c::value_boolean(false);
            result_map["role"] = xmlrpc_c::value_int(-1); // Rol inválido para indicar fallo
            Logger::getInstance().log(LogLevel::WARNING, "Failed login attempt", username, clientIp);
        }
        *retvalP = xmlrpc_c::value_struct(result_map);
    }
};

// --- Método para añadir usuarios (solo para administradores) ---
class RobotUserAddMethod : public AuthenticatedMethod {
public:
    RobotUserAddMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        // Firma: bool user_add(string adminUser, string adminPass, string newUser, string newPass, int newRole)
        this->_signature = "b:ssssi";
        this->_name = "robot.user_add";
        this->_help = "Adds a new user. Requires administrator privileges.";
    }

    // Implementamos el método virtual puro.
    void executeAuthenticated(xmlrpc_c::paramList const& paramList, 
                              xmlrpc_c::value* const retvalP, 
                              UserNamespace::User& adminUser,
                              const std::string& clientIp) override { // Lanza xmlrpc_c::fault
        // 1. Verificar que el usuario autenticado es un administrador.
        if (adminUser.getRole() != UserRole::ADMIN) {
            throw PermissionDeniedException("Solo los administradores pueden añadir usuarios.");
        }

        // 2. Extraer los parámetros del nuevo usuario. Lanza xmlrpc_c::fault si los params son incorrectos
        // Los parámetros del admin (0 y 1) ya fueron usados para la autenticación.
        std::string const newUsername(paramList.getString(2));
        std::string const newPassword(paramList.getString(3));
        int         const newRoleInt(paramList.getInt(4));
        paramList.verifyEnd(5);

        UserRole newRole = static_cast<UserRole>(newRoleInt);

        // 3. Llamar al servicio para crear el usuario.
        bool success = authService.createUser(newUsername, newPassword, newRole); // Lanza DatabaseException en caso de fallo

        // 4. Devolver el resultado.
        robot.recordOrder(adminUser.getUsername(), "user_add", "Added user: " + newUsername);
        *retvalP = xmlrpc_c::value_boolean(success);
    }
};


// --- Método para obtener ayuda contextual ---
class HelpMethod : public AuthenticatedMethod {
public:
    HelpMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "S:ss"; // Devuelve un struct: {command: help_string, ...}
        this->_name = "robot.help";
        this->_help = "Lists available commands and their syntax based on user role.";
    }

    void executeAuthenticated(xmlrpc_c::paramList const& paramList,
                              xmlrpc_c::value* const retvalP,
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        
        paramList.verifyEnd(2);
        std::map<std::string, xmlrpc_c::value> helpMap;

        // --- Comandos para Operadores (y Admins) ---
        helpMap["estado"] = xmlrpc_c::value_string("Muestra el estado actual del robot.");
        helpMap["motores_on"] = xmlrpc_c::value_string("Activa los motores del robot.");
        helpMap["motores_off"] = xmlrpc_c::value_string("Desactiva los motores del robot.");
        helpMap["mover <x> <y> <z> <vel>"] = xmlrpc_c::value_string("Mueve el efector a una posición (x,y,z) a una velocidad (vel).");
        helpMap["efector_on"] = xmlrpc_c::value_string("Activa el efector final.");
        helpMap["efector_off"] = xmlrpc_c::value_string("Desactiva el efector final.");
        helpMap["lista_tareas"] = xmlrpc_c::value_string("Muestra las tareas predefinidas disponibles.");
        helpMap["ejecutar_tarea <id_tarea>"] = xmlrpc_c::value_string("Ejecuta una tarea predefinida por su ID.");
        helpMap["aprender_inicio <id> <nombre>"] = xmlrpc_c::value_string("Inicia el modo de aprendizaje de trayectoria.");
        helpMap["aprender_fin"] = xmlrpc_c::value_string("Finaliza el aprendizaje y guarda la tarea.");
        helpMap["reporte"] = xmlrpc_c::value_string("Muestra un reporte de actividad de la sesión actual.");
        helpMap["salir"] = xmlrpc_c::value_string("Cierra la consola del cliente.");
        
        // --- Comandos solo para Administradores ---
        if (user.getRole() == UserRole::ADMIN) {
            helpMap["conectar"] = xmlrpc_c::value_string("Establece la conexión con el hardware del robot.");
            helpMap["desconectar"] = xmlrpc_c::value_string("Cierra la conexión con el hardware del robot.");
            helpMap["user_add <user> <pass> <role>"] = xmlrpc_c::value_string("Añade un nuevo usuario (role: 0=ADMIN, 1=OPERATOR).");
            helpMap["report_admin [filtro] [val]"] = xmlrpc_c::value_string("Muestra reporte admin con filtros [usuario ó resultado] [valor].");
            helpMap["report_log [filtro] [val]"] = xmlrpc_c::value_string("Muestra reporte de log con filtros [usuario ó nivel] [valor].");
        }

        *retvalP = xmlrpc_c::value_struct(helpMap);
    }
};


// --- Método para obtener el reporte del operador ---
class GetReportMethod : public AuthenticatedMethod {
public:
    GetReportMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "S:ss"; // Devuelve un struct
        this->_name = "robot.getReport";
        this->_help = "Generates a report for the current user.";
    }

    void executeAuthenticated(xmlrpc_c::paramList const& paramList,
                              xmlrpc_c::value* const retvalP,
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        ReportGenerator reportGenerator; // Create an instance of ReportGenerator
        paramList.verifyEnd(2);
        *retvalP = reportGenerator.generateOperatorReport(robot, user);
    }
};


// --- Método para obtener el reporte del administrador ---
class GetAdminReportMethod : public AuthenticatedMethod {
public:
    GetAdminReportMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "S:ssss"; // struct getAdminReport(string user, string pass, string filterKey, string filterValue)
        this->_name = "robot.getAdminReport";
        this->_help = "Generates a report for the administrator, with optional filtering.";
    }

    void executeAuthenticated(xmlrpc_c::paramList const& paramList,
                              xmlrpc_c::value* const retvalP,
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        if (user.getRole() != UserRole::ADMIN) {
            throw xmlrpc_c::fault("Permission denied: Only administrators can access this report.", xmlrpc_c::fault::CODE_INTERNAL);
        }

        std::string const filterKey(paramList.getString(2));
        std::string const filterValue(paramList.getString(3));
        paramList.verifyEnd(4);

        std::map<std::string, std::string> filters;
        if (!filterKey.empty()) {
            filters[filterKey] = filterValue;
        }
        
        ReportGenerator reportGenerator;
        *retvalP = reportGenerator.generateAdminReport(robot, filters);
    }
};


// --- Método para obtener el reporte del log del servidor ---
class GetLogReportMethod : public AuthenticatedMethod {
public:
    GetLogReportMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "S:ssss"; // struct getLogReport(string user, string pass, string filterKey, string filterValue)
        this->_name = "robot.getLogReport";
        this->_help = "Generates a server log report, with optional filtering by 'username' or 'level'.";
    }

    void executeAuthenticated(xmlrpc_c::paramList const& paramList,
                              xmlrpc_c::value* const retvalP,
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        if (user.getRole() != UserRole::ADMIN) {
            throw xmlrpc_c::fault("Permission denied: Only administrators can access this report.", xmlrpc_c::fault::CODE_INTERNAL);
        }

        std::string const filterKey(paramList.getString(2));
        std::string const filterValue(paramList.getString(3));
        paramList.verifyEnd(4);

        std::map<std::string, std::string> filters;
        if (!filterKey.empty()) {
            filters[filterKey] = filterValue;
        }
        
        ReportGenerator reportGenerator;
        *retvalP = reportGenerator.generateLogReport(filters);
    }
};


// --- Método para listar las tareas disponibles ---
class ListTasksMethod : public AuthenticatedMethod {
public:
    ListTasksMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "A:ss"; // Devuelve un array
        this->_name = "robot.listTasks";
        this->_help = "Lists all available pre-defined tasks.";
    }

    void executeAuthenticated(xmlrpc_c::paramList const& paramList,
                              xmlrpc_c::value* const retvalP,
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        paramList.verifyEnd(2);
        
        std::vector<xmlrpc_c::value> tasksVector;
        const auto& availableTasks = taskManager.getAvailableTasks();

        for (const auto& task : availableTasks) {
            std::map<std::string, xmlrpc_c::value> taskMap;
            taskMap["id"] = xmlrpc_c::value_string(task.id);
            taskMap["name"] = xmlrpc_c::value_string(task.name);
            taskMap["description"] = xmlrpc_c::value_string(task.description);
            tasksVector.push_back(xmlrpc_c::value_struct(taskMap));
        }

        *retvalP = xmlrpc_c::value_array(tasksVector);
    }
};

// --- Método para ejecutar una tarea por ID ---
class ExecuteTaskMethod : public AuthenticatedMethod {
public:
    ExecuteTaskMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "b:sss"; // boolean executeTask(user, pass, taskId)
        this->_name = "robot.executeTask";
        this->_help = "Executes a pre-defined task by its ID.";
    }

    void executeAuthenticated(xmlrpc_c::paramList const& paramList,
                              xmlrpc_c::value* const retvalP,
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        std::string const taskId(paramList.getString(2));
        paramList.verifyEnd(3);

        auto taskOpt = taskManager.getTaskById(taskId);
        if (!taskOpt) {
            throw xmlrpc_c::fault("Task with ID '" + taskId + "' not found.", xmlrpc_c::fault::CODE_INTERNAL);
        }

        // Registramos el inicio de la tarea
        robot.recordOrder(user.getUsername(), "execute_task", "Executing task: " + taskId);

        // Ejecutamos cada comando G-Code de la tarea
        for (const auto& gcode : taskOpt->gcode) {
            robot.sendRawGCode(gcode);
            // Podríamos añadir un pequeño delay si es necesario entre comandos
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        *retvalP = xmlrpc_c::value_boolean(true);
    }
};

// --- Método para añadir una nueva tarea ---
class AddTaskMethod : public AuthenticatedMethod {
public:
    AddTaskMethod(AuthenticationServiceNamespace::AuthenticationService& auth, RobotNamespace::Robot& r, TaskManager& tm)
        : AuthenticatedMethod(auth, r, tm) {
        this->_signature = "b:ssssA"; // bool addTask(user, pass, id, name, gcode_array)
        this->_name = "robot.addTask";
        this->_help = "Adds a new learned task to the system.";
    }

    void executeAuthenticated(xmlrpc_c::paramList const& paramList,
                              xmlrpc_c::value* const retvalP,
                              UserNamespace::User& user,
                              const std::string& clientIp) override {
        std::string const taskId(paramList.getString(2));
        std::string const taskName(paramList.getString(3));
        xmlrpc_c::value_array const gcodeArray(paramList.getArray(4));
        paramList.verifyEnd(5);

        std::vector<xmlrpc_c::value> const gcodeVector(gcodeArray.vectorValueValue());

        Task newTask;
        newTask.id = taskId;
        newTask.name = taskName;
        newTask.description = "Tarea aprendida por el usuario " + user.getUsername();

        for (const auto& gcodeValue : gcodeVector) {
            newTask.gcode.push_back(xmlrpc_c::value_string(gcodeValue).cvalue());
        }

        bool success = taskManager.addTask(newTask);
        if (!success) {
            throw xmlrpc_c::fault("Failed to add task. ID might already exist.", xmlrpc_c::fault::CODE_INTERNAL);
        }

        robot.recordOrder(user.getUsername(), "add_task", "Nueva tarea aprendida: " + taskId);
        *retvalP = xmlrpc_c::value_boolean(true);
    }
};


void RpcServiceHandlerNamespace::RpcServiceHandler::registerMethods(xmlrpc_c::registry &registry) {
    // Registramos el método de autenticación.
    registry.addMethod("robot.authenticate", new RobotAuthenticateMethod(authService));
    // Registramos los demás métodos protegidos.
    registry.addMethod("robot.connect", new RobotConnectMethod(authService, robot, taskManager));
    registry.addMethod("robot.disconnect", new RobotDisconnectMethod(authService, robot, taskManager));
    registry.addMethod("robot.user_add", new RobotUserAddMethod(authService, robot, taskManager));
    registry.addMethod("robot.getStatus", new RobotGetStatusMethod(authService, robot, taskManager));
    registry.addMethod("robot.move", new RobotMoveMethod(authService, robot, taskManager));
    registry.addMethod("robot.moveDefaultSpeed", new RobotMoveDefaultSpeedMethod(authService, robot, taskManager));
    registry.addMethod("robot.enableMotors", new RobotEnableMotorsMethod(authService, robot, taskManager));
    registry.addMethod("robot.disableMotors", new RobotDisableMotorsMethod(authService, robot, taskManager));
    registry.addMethod("robot.setCoordinateMode", new SetCoordinateModeMethod(authService, robot, taskManager));
    registry.addMethod("robot.setEffector", new RobotSetEffectorMethod(authService, robot, taskManager));
    registry.addMethod("robot.help", new HelpMethod(authService, robot, taskManager));
    registry.addMethod("robot.getReport", new GetReportMethod(authService, robot, taskManager));
    registry.addMethod("robot.getAdminReport", new GetAdminReportMethod(authService, robot, taskManager));
    registry.addMethod("robot.getLogReport", new GetLogReportMethod(authService, robot, taskManager));
    registry.addMethod("robot.listTasks", new ListTasksMethod(authService, robot, taskManager));
    registry.addMethod("robot.executeTask", new ExecuteTaskMethod(authService, robot, taskManager));
    registry.addMethod("robot.addTask", new AddTaskMethod(authService, robot, taskManager));
}