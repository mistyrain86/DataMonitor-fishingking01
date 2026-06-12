#pragma once
#include <string>
#include "OrderStatus.h"

struct Order {
    std::string id;
    std::string sampleId;
    std::string customerName;
    int         quantity           = 0;
    OrderStatus status             = OrderStatus::RESERVED;
    std::string orderedAt;
    int         requiredProduction = 0;
};
