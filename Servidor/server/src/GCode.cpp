#include "GCode.h"
#include <sstream>
#include "Utils.h"
// Constructors/Destructors


GCodeNamespace::GCode::GCode()
{
}

GCodeNamespace::GCode::~GCode()
{
}

// Inicialización de las constantes estáticas definidas en el .h
constexpr double GCodeNamespace::GCode::MAX_REACH;

std::string GCodeNamespace::GCode::generateMoveCommand(double x, double y, double z, double speed) {

    // if (!GCode::isReachable(x, y, z)) {
    //     // Si el punto no es alcanzable, devolvemos un comentario G-Code
    //     // indicando el error. Los comentarios inician con punto y coma (;).
    //     return "; ERROR: COORDENADA FUERA DEL ESPACIO DE TRABAJO. MOVIMIENTO BLOQUEADO.";
    // }


    std::stringstream ss;
    // G1 es el comando para movimiento lineal. F es para la velocidad (Feed rate).
    ss << "G1 "
       << "X" << std::fixed << std::setprecision(3) << x << " "
       << "Y" << std::fixed << std::setprecision(3) << y << " "
       << "Z" << std::fixed << std::setprecision(3) << z << " "
       << "E" << std::fixed << std::setprecision(1) << speed;
    return ss.str();
}


std::string GCodeNamespace::GCode::generateMoveCommand(double x, double y, double z) {
    // Llama a la otra versión de la función con la velocidad por defecto.
    return generateMoveCommand(x, y, z, DEFAULT_SPEED);
}


// bool GCodeNamespace::GCode::isReachable(double x, double y, double z) {
//     // 1. Calcular la distancia horizontal (R) al origen (proyección en el plano XY)
//     // R = sqrt(X^2 + Y^2)
//     double r = std::sqrt(x * x + y * y);

//     // 2. Restricción Radial (Alcance Máximo)
//     if (r > MAX_REACH) {
//         // El punto está fuera del radio máximo de 240mm.
//         throw RobotException("ERROR de Alcance: La distancia horizontal (" + double_a_string_con_precision(r, 2) + "mm) excede el alcance maximo (" + double_a_string_con_precision(MAX_REACH, 2) + "mm).");
//         return false;
//     }

//     // 3. Restricción Radial (Alcance Mínimo)
//     // Se establece un radio mínimo para evitar singularidades o auto-colisión en la base.
//     const double MIN_OPERATIONAL_RADIUS = 135.5; 
//     if (r < MIN_OPERATIONAL_RADIUS) {
//         throw RobotException("ADVERTENCIA: Punto demasiado cerca del centro (" + double_a_string_con_precision(r, 2) + "mm). Puede causar singularidad. (Movimiento permitido, pero con precaucion).");
//     }

//     // 4. Restricción Vertical (Altura Z)
//     // Rango Z de ejemplo: -50mm (por debajo de la base) a 240mm (totalmente extendido verticalmente).
//     const double MIN_Z = 36.0; 
//     const double MAX_Z = 256.0; 

//     if (z < MIN_Z) {
//         throw RobotException("ERROR de Altura Z: La altura (" + double_a_string_con_precision(z, 2) + "mm es demasiado baja (Min: " + double_a_string_con_precision(MIN_Z, 2) + "mm).");
//         return false;
//     }
    
//     if (z > MAX_Z) {
//         throw RobotException("ERROR de Altura Z: La altura (" + double_a_string_con_precision(z, 2) + "mm) excede el maximo vertical (Max: " + double_a_string_con_precision(MAX_Z, 2) + "mm).");
//         return false;
//     }

//     // Si pasa todas las pruebas, es alcanzable.
//     return true;
// }