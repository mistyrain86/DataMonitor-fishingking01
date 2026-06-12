#include "MonitorController.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

MonitorController::MonitorController(SampleRepository& sampleRepo, OrderRepository& orderRepo)
    : sampleRepo_(sampleRepo), orderRepo_(orderRepo) {}

MonitorData MonitorController::refresh() {
    sampleRepo_.load();
    orderRepo_.load();

    auto samples = sampleRepo_.findAll();
    auto orders  = orderRepo_.findAll();

    MonitorData data;
    data.hasData        = !samples.empty() || !orders.empty();
    data.orderSummary   = buildOrderSummary(orders);
    data.sampleStatuses = buildSampleStatuses(samples, orders);
    data.alerts         = buildAlerts(data.orderSummary, data.sampleStatuses);
    data.lastUpdated    = currentTimestamp();
    return data;
}

OrderSummary MonitorController::buildOrderSummary(const std::vector<Order>& orders) const {
    OrderSummary summary;
    for (const auto& o : orders) {
        switch (o.status) {
            case OrderStatus::RESERVED:  ++summary.reservedCount;  break;
            case OrderStatus::CONFIRMED: ++summary.confirmedCount; break;
            case OrderStatus::PRODUCING: ++summary.producingCount; break;
            case OrderStatus::RELEASE:   ++summary.releaseCount;   break;
            case OrderStatus::REJECTED:  ++summary.rejectedCount;  break;
        }
    }
    summary.totalCount = (int)orders.size();
    return summary;
}

std::vector<SampleMonitorStatus> MonitorController::buildSampleStatuses(
    const std::vector<Sample>& samples,
    const std::vector<Order>&  orders) const
{
    std::vector<SampleMonitorStatus> statuses;
    for (const auto& s : samples) {
        SampleMonitorStatus st;
        st.sample     = s;
        st.isLowStock = (s.quantity <= LOW_STOCK_THRESHOLD);
        for (const auto& o : orders) {
            if (o.sampleId != s.id) continue;
            switch (o.status) {
                case OrderStatus::RESERVED:  st.reservedQty  += o.quantity; break;
                case OrderStatus::CONFIRMED: st.confirmedQty += o.quantity; break;
                case OrderStatus::PRODUCING:
                    st.producingQty         += o.quantity;
                    st.requiredProductionTotal += o.requiredProduction;
                    break;
                case OrderStatus::RELEASE:   st.releasedQty  += o.quantity; break;
                default: break;
            }
        }
        statuses.push_back(std::move(st));
    }
    return statuses;
}

std::vector<Alert> MonitorController::buildAlerts(
    const OrderSummary& summary,
    const std::vector<SampleMonitorStatus>& statuses) const
{
    std::vector<Alert> alerts;

    // 재고 부족 경고
    for (const auto& st : statuses) {
        if (st.isLowStock) {
            Alert a;
            a.level   = Alert::Level::WARNING;
            a.title   = "재고부족";
            a.message = "[" + st.sample.id + "] " + st.sample.name
                      + " : 현재 재고 " + std::to_string(st.sample.quantity)
                      + "개 (임계값 " + std::to_string(LOW_STOCK_THRESHOLD) + "개 이하)";
            alerts.push_back(std::move(a));
        }
    }

    // PRODUCING 주문 과다 경고
    if (summary.producingCount >= PRODUCING_ALERT_THRESHOLD) {
        Alert a;
        a.level   = Alert::Level::WARNING;
        a.title   = "생산대기";
        a.message = "생산 중인 주문이 " + std::to_string(summary.producingCount)
                  + "건입니다. (기준: " + std::to_string(PRODUCING_ALERT_THRESHOLD) + "건 이상)";
        alerts.push_back(std::move(a));
    }

    // RESERVED 대기 주문 과다 안내
    if (summary.reservedCount >= RESERVED_ALERT_THRESHOLD) {
        Alert a;
        a.level   = Alert::Level::INFO;
        a.title   = "승인대기";
        a.message = "승인 대기 주문이 " + std::to_string(summary.reservedCount)
                  + "건 있습니다. 검토가 필요합니다.";
        alerts.push_back(std::move(a));
    }

    // 생산 필요량 정보
    int totalRequired = 0;
    for (const auto& st : statuses)
        totalRequired += st.requiredProductionTotal;
    if (totalRequired > 0) {
        Alert a;
        a.level   = Alert::Level::INFO;
        a.title   = "생산예정";
        a.message = "총 생산 예정 수량: " + std::to_string(totalRequired) + "개";
        alerts.push_back(std::move(a));
    }

    return alerts;
}

std::string MonitorController::currentTimestamp() const {
    auto now   = std::chrono::system_clock::now();
    auto time  = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &time);
#else
    localtime_r(&time, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::vector<Order> MonitorController::getOrdersByStatus(OrderStatus status) const {
    return orderRepo_.findByStatus(status);
}

std::vector<Order> MonitorController::getOrdersBySampleId(const std::string& sampleId) const {
    return orderRepo_.findBySampleId(sampleId);
}

std::vector<Sample> MonitorController::getAllSamples() const {
    return sampleRepo_.findAll();
}

std::vector<Order> MonitorController::getAllOrders() const {
    return orderRepo_.findAll();
}
