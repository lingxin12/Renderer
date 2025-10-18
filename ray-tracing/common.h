#pragma once
#include <iostream>
#include <cmath>
#include <limits>
#include <random>
#include <ctime>

namespace lxrt {

// Constants

const double kinfinity = std::numeric_limits<double>::infinity();
const double kpi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * kpi / 180.0;
}

inline double random_double() {
    // return a random real in [0,1)
    static std::mt19937 e(time(0));
    static std::uniform_real_distribution<double> dis(0, 1);
    return dis(e);
}

inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

inline int random_int(int min, int max) {
    return static_cast<int>(random_double(min, max + 1));
}

} // namespace lxrt