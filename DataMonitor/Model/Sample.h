#pragma once
#include <string>

struct Sample {
    std::string id;
    std::string name;
    int         quantity    = 0;
    double      yield       = 0.0;
    double      cycleTime   = 0.0;
    std::string registeredAt;
};
