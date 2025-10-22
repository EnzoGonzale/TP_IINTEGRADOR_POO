#include "GCode.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cmath>
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

    if (!GCode::isReachable(x, y, z)) {
        // Si el punto no es alcanzable, devolvemos un comentario G-Code
        // indicando el error. Los comentarios inician con punto y coma (;).
        return "; ERROR: COORDENADA FUERA DEL ESPACIO DE TRABAJO. MOVIMIENTO BLOQUEADO.";
    }


    std::stringstream ss;
    // G1 es el comando para movimiento lineal. F es para la velocidad (Feed rate).
    ss << "G1 "
       << "X" << std::fixed << std::setprecision(3) << x << " "
       << "Y" << std::fixed << std::setprecision(3) << y << " "
       << "Z" << std::fixed << std::setprecision(3) << z << " "
       << "F" << std::fixed << std::setprecision(1) << speed;
    return ss.str();
}


bool GCodeNamespace::GCode::isReachable(double x, double y, double z) {
    // 1. Calcular la distancia horizontal (R) al origen (proyección en el plano XY)
    // R = sqrt(X^2 + Y^2)
    double r = std::sqrt(x * x + y * y);

    // 2. Restricción Radial (Alcance Máximo)
    if (r > MAX_REACH) {
        // El punto está fuera del radio máximo de 240mm.
        std::cerr << "ERROR de Alcance: La distancia horizontal (" << std::fixed << std::setprecision(2) << r 
                  << "mm) excede el alcance maximo (" << MAX_REACH << "mm)." << std::endl;
        return false;
    }

    // 3. Restricción Radial (Alcance Mínimo)
    // Se establece un radio mínimo para evitar singularidades o auto-colisión en la base.
    const double MIN_OPERATIONAL_RADIUS = 1.0; 
    if (r < MIN_OPERATIONAL_RADIUS) {
        std::cerr << "ADVERTENCIA: Punto demasiado cerca del centro (" << std::fixed << std::setprecision(2) << r 
                  << "mm). Puede causar singularidad. (Movimiento permitido, pero con precaucion)." << std::endl;
    }

    // 4. Restricción Vertical (Altura Z)
    // Rango Z de ejemplo: -50mm (por debajo de la base) a 240mm (totalmente extendido verticalmente).
    const double MIN_Z = -50.0; 
    const double MAX_Z = MAX_REACH; 

    if (z < MIN_Z) {
        std::cerr << "ERROR de Altura Z: La altura (" << std::fixed << std::setprecision(2) << z 
                  << "mm) es demasiado baja (Min: " << MIN_Z << "mm)." << std::endl;
        return false;
    }
    
    if (z > MAX_Z) {
        std::cerr << "ERROR de Altura Z: La altura (" << std::fixed << std::setprecision(2) << z 
                  << "mm) excede el maximo vertical (Max: " << MAX_Z << "mm)." << std::endl;
        return false;
    }

    // Si pasa todas las pruebas, es alcanzable.
    return true;
}
