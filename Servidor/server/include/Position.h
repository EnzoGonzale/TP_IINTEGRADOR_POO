
#ifndef POSITION_H
#define POSITION_H
// Un struct es perfecto para agrupar datos simples.
// Por defecto, sus miembros son públicos.
struct Position {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
 
  // Constructor para facilitar la creación
  Position(double x_val = 0.0, double y_val = 0.0, double z_val = 0.0)
    : x(x_val), y(y_val), z(z_val) {}
};
#endif // POSITION_H
