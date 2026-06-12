#pragma once
#include <string>
#include "../Controller/MonitorController.h"

class DashboardView {
public:
    explicit DashboardView(MonitorController& controller, int refreshSeconds = 5);

    void run();

private:
    MonitorController& controller_;
    int                refreshSeconds_;

    // 메인 화면 렌더링
    void renderDashboard(const MonitorData& data) const;
    void renderHeader(const MonitorData& data) const;
    void renderOrderSummary(const OrderSummary& summary) const;
    void renderSampleStatus(const std::vector<SampleMonitorStatus>& statuses) const;
    void renderAlerts(const std::vector<Alert>& alerts) const;
    void renderFooter() const;

    // 상세 화면
    void showOrderDetail(OrderStatus status) const;
    void showSampleDetail() const;
    void showAllOrders() const;
    void showSettings();

    // 유틸리티
    std::string makeBar(int value, int maxVal, int barWidth = 20) const;
    std::string centerText(const std::string& text, int width) const;
    void clearScreen() const;
    void enableAnsi() const;
    void printLine(const std::string& content, int width = 68) const;
    void printSeparator(int width = 68) const;
    void printDoubleLine(int width = 68) const;
    void waitEnter() const;
};
