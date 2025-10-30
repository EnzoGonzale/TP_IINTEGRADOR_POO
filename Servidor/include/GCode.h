
#ifndef GCODE_H
#define GCODE_H

#include <string>

namespace GCodeNamespace
{


/// 
/// class GCode

class GCode
{
public:
  // Constructors/Destructors  



  /// 
  /// Empty Constructor
  GCode();

  /// 
  /// Empty Destructor
  virtual ~GCode();

  // --- Static Utility Methods ---

  /// @brief Genera un comando G-Code para un movimiento lineal (G1).
  /// @param x Coordenada X del destino.
  /// @param y Coordenada Y del destino.
  /// @param z Coordenada Z del destino.
  /// @param speed Velocidad de avance (feed rate) para el movimiento.
  /// @return Una cadena de texto con el comando G-Code formateado.
  static std::string generateMoveCommand(double x, double y, double z, double speed);


  /// @brief Genera un comando G-Code para un movimiento lineal (G1) con velocidad por defecto.
  /// @param x Coordenada X del destino.
  /// @param y Coordenada Y del destino.
  /// @param z Coordenada Z del destino.
  /// @return Una cadena de texto con el comando G-Code formateado.
  static std::string generateMoveCommand(double x, double y, double z);


  /// @param x Coordenada X.
  /// @param y Coordenada Y.
  /// @param z Coordenada Z.
  /// @return true si es alcanzable, false en caso contrario.
  static bool isReachable(double x, double y, double z);


private:
  // --- Constantes Estáticas del Brazo ---
  // Al ser estáticas, pertenecen a la clase y no a un objeto.
  // Longitud de los eslabones del brazo (dado por el usuario: 120 mm)
    static constexpr double L1 = 120.0;
    static constexpr double L2 = 120.0;
    
    // Alcance máximo del brazo (L1 + L2)
    static constexpr double MAX_REACH = L1 + L2; // 240.0 mm

    // Velocidad por defecto para movimientos si no se especifica una.
    static constexpr double DEFAULT_SPEED = 0.0; // 2000 mm/min

};

} // namespace GCodeNamespace

#endif // GCODE_H
