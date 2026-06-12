#pragma once
#include <string>
#include <vector>
#include "../Model/Sample.h"
#include "../Model/Order.h"
#include "../Model/SampleRepository.h"
#include "../Model/OrderRepository.h"

// 임계값 설정
constexpr int LOW_STOCK_THRESHOLD      = 10;   // 재고 부족 경고 기준
constexpr int PRODUCING_ALERT_THRESHOLD = 3;   // PRODUCING 주문 과다 경고 기준
constexpr int RESERVED_ALERT_THRESHOLD  = 5;   // RESERVED 대기 경고 기준

struct OrderSummary {
    int reservedCount  = 0;
    int confirmedCount = 0;
    int producingCount = 0;
    int releaseCount   = 0;
    int rejectedCount  = 0;
    int totalCount     = 0;
};

struct SampleMonitorStatus {
    Sample sample;
    int reservedQty          = 0;
    int confirmedQty         = 0;
    int producingQty         = 0;
    int releasedQty          = 0;
    int requiredProductionTotal = 0;
    bool isLowStock          = false;
};

struct Alert {
    enum class Level { INFO, WARNING, CRITICAL };
    Level       level;
    std::string title;
    std::string message;
};

struct MonitorData {
    OrderSummary                    orderSummary;
    std::vector<SampleMonitorStatus> sampleStatuses;
    std::vector<Alert>              alerts;
    std::string                     lastUpdated;
    bool                            hasData = false;
};

class MonitorController {
public:
    MonitorController(SampleRepository& sampleRepo, OrderRepository& orderRepo);

    MonitorData refresh();

    // 상세 조회
    std::vector<Order>  getOrdersByStatus(OrderStatus status) const;
    std::vector<Order>  getOrdersBySampleId(const std::string& sampleId) const;
    std::vector<Sample> getAllSamples() const;
    std::vector<Order>  getAllOrders() const;

private:
    SampleRepository& sampleRepo_;
    OrderRepository&  orderRepo_;

    OrderSummary buildOrderSummary(const std::vector<Order>& orders) const;
    std::vector<SampleMonitorStatus> buildSampleStatuses(
        const std::vector<Sample>& samples,
        const std::vector<Order>&  orders) const;
    std::vector<Alert> buildAlerts(
        const OrderSummary& summary,
        const std::vector<SampleMonitorStatus>& statuses) const;
    std::string currentTimestamp() const;
};
