#include "OrderRepository.h"
#include "../Util/JsonReader.h"
#include <stdexcept>

OrderRepository::OrderRepository(std::string filePath)
    : filePath_(std::move(filePath)) {
    load();
}

bool OrderRepository::load() {
    auto arr = JsonReader::readArray(filePath_, "orders");
    orders_.clear();
    for (const auto& obj : arr) {
        Order o;
        if (auto it = obj.find("id");                  it != obj.end()) o.id                  = it->second;
        if (auto it = obj.find("sampleId");            it != obj.end()) o.sampleId            = it->second;
        if (auto it = obj.find("customerName");        it != obj.end()) o.customerName        = it->second;
        if (auto it = obj.find("quantity");            it != obj.end()) o.quantity            = std::stoi(it->second);
        if (auto it = obj.find("status");              it != obj.end()) o.status              = statusFromString(it->second);
        if (auto it = obj.find("orderedAt");           it != obj.end()) o.orderedAt           = it->second;
        if (auto it = obj.find("requiredProduction");  it != obj.end()) o.requiredProduction  = std::stoi(it->second);
        if (!o.id.empty()) orders_.push_back(std::move(o));
    }
    return !orders_.empty();
}

std::vector<Order> OrderRepository::findAll() const {
    return orders_;
}

std::vector<Order> OrderRepository::findByStatus(OrderStatus status) const {
    std::vector<Order> result;
    for (const auto& o : orders_)
        if (o.status == status) result.push_back(o);
    return result;
}

std::vector<Order> OrderRepository::findBySampleId(const std::string& sampleId) const {
    std::vector<Order> result;
    for (const auto& o : orders_)
        if (o.sampleId == sampleId) result.push_back(o);
    return result;
}

std::optional<Order> OrderRepository::findById(const std::string& id) const {
    for (const auto& o : orders_)
        if (o.id == id) return o;
    return std::nullopt;
}
