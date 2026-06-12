#pragma once
#include <vector>
#include <optional>
#include <string>
#include "Order.h"

class OrderRepository {
public:
    explicit OrderRepository(std::string filePath);

    std::vector<Order>   findAll() const;
    std::vector<Order>   findByStatus(OrderStatus status) const;
    std::vector<Order>   findBySampleId(const std::string& sampleId) const;
    std::optional<Order> findById(const std::string& id) const;
    bool                 load();

    const std::string& filePath() const { return filePath_; }

private:
    std::string        filePath_;
    std::vector<Order> orders_;
};
