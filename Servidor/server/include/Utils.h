#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <iomanip>

inline std::string double_a_string_con_precision(double valor, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << valor;
    return oss.str();
}

#endif // UTILS_H