#pragma once
#include <string>

enum class OrderStatus {
    RESERVED,
    REJECTED,
    PRODUCING,
    CONFIRMED,
    RELEASE
};

inline std::string toString(OrderStatus status) {
    switch (status) {
        case OrderStatus::RESERVED:  return "RESERVED";
        case OrderStatus::REJECTED:  return "REJECTED";
        case OrderStatus::PRODUCING: return "PRODUCING";
        case OrderStatus::CONFIRMED: return "CONFIRMED";
        case OrderStatus::RELEASE:   return "RELEASE";
        default: return "UNKNOWN";
    }
}

inline OrderStatus statusFromString(const std::string& s) {
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "RELEASE")   return OrderStatus::RELEASE;
    return OrderStatus::RESERVED;
}
