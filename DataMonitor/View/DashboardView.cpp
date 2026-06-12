#include "DashboardView.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
#include <conio.h>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

// ANSI 색상 코드
namespace Ansi {
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string DIM     = "\033[2m";
    const std::string RED     = "\033[91m";
    const std::string GREEN   = "\033[92m";
    const std::string YELLOW  = "\033[93m";
    const std::string BLUE    = "\033[94m";
    const std::string MAGENTA = "\033[95m";
    const std::string CYAN    = "\033[96m";
    const std::string WHITE   = "\033[97m";
    const std::string BG_RED  = "\033[41m";
    const std::string BG_BLUE = "\033[44m";
}

DashboardView::DashboardView(MonitorController& controller, int refreshSeconds)
    : controller_(controller), refreshSeconds_(refreshSeconds) {}

void DashboardView::enableAnsi() const {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

void DashboardView::clearScreen() const {
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\033[2J\033[H";
#endif
}

void DashboardView::printLine(const std::string& content, int width) const {
    std::cout << "│ " << content;
    // 콘솔 출력용 단순 패딩 (UTF-8 멀티바이트 주의로 고정폭 미사용)
    std::cout << "\n";
}

void DashboardView::printSeparator(int width) const {
    std::cout << "├";
    for (int i = 0; i < width; ++i) std::cout << "─";
    std::cout << "┤\n";
}

void DashboardView::printDoubleLine(int width) const {
    std::cout << "└";
    for (int i = 0; i < width; ++i) std::cout << "─";
    std::cout << "┘\n";
}

std::string DashboardView::makeBar(int value, int maxVal, int barWidth) const {
    if (maxVal <= 0) return std::string(barWidth, ' ');
    int filled = (int)((double)value / maxVal * barWidth);
    filled = std::min(filled, barWidth);
    std::string bar;
    for (int i = 0; i < filled; ++i) bar += "█";
    for (int i = filled; i < barWidth; ++i) bar += "░";
    return bar;
}

std::string DashboardView::centerText(const std::string& text, int width) const {
    int padding = (width - (int)text.size()) / 2;
    if (padding < 0) padding = 0;
    return std::string(padding, ' ') + text;
}

void DashboardView::run() {
    enableAnsi();

    while (true) {
        auto data = controller_.refresh();
        renderDashboard(data);

        // refreshSeconds 동안 키 입력 대기
        for (int elapsed = 0; elapsed < refreshSeconds_ * 10; ++elapsed) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (_kbhit()) {
                char ch = (char)_getch();
                ch = (char)toupper((unsigned char)ch);
                if (ch == 'Q') {
                    std::cout << Ansi::YELLOW << "\n모니터링을 종료합니다.\n" << Ansi::RESET;
                    return;
                }
                if (ch == 'R') break; // 즉시 새로고침

                if (ch == '1') { showOrderDetail(OrderStatus::RESERVED);  break; }
                if (ch == '2') { showOrderDetail(OrderStatus::CONFIRMED); break; }
                if (ch == '3') { showOrderDetail(OrderStatus::PRODUCING); break; }
                if (ch == '4') { showSampleDetail(); break; }
                if (ch == '5') { showAllOrders();    break; }
                if (ch == 'S') { showSettings();     break; }
            }
        }
    }
}

void DashboardView::renderDashboard(const MonitorData& data) const {
    clearScreen();
    renderHeader(data);
    renderOrderSummary(data.orderSummary);
    printSeparator();
    renderSampleStatus(data.sampleStatuses);
    printSeparator();
    renderAlerts(data.alerts);
    renderFooter();
}

void DashboardView::renderHeader(const MonitorData& data) const {
    std::cout << Ansi::CYAN << Ansi::BOLD;
    std::cout << "┌";
    for (int i = 0; i < 68; ++i) std::cout << "─";
    std::cout << "┐\n";
    std::cout << "│" << centerText("반도체 시료 관리 - 데이터 모니터링 대시보드", 69) << "│\n";
    std::cout << "│" << centerText("최종 갱신: " + data.lastUpdated, 69) << "│\n";
    std::cout << Ansi::RESET;
    printSeparator();

    if (!data.hasData) {
        std::cout << Ansi::YELLOW;
        printLine("  데이터 없음. data/samples.json, data/orders.json 파일을 확인하세요.");
        std::cout << Ansi::RESET;
        printSeparator();
    }
}

void DashboardView::renderOrderSummary(const OrderSummary& summary) const {
    int maxCount = std::max({summary.reservedCount, summary.confirmedCount,
                             summary.producingCount, summary.releaseCount,
                             summary.rejectedCount, 1});

    std::cout << "│ " << Ansi::BOLD << "[ 주문 현황 요약 ]" << Ansi::RESET
              << "  (총 " << summary.totalCount << "건)\n";

    auto printRow = [&](const std::string& label, int count, const std::string& color, const std::string& flag="") {
        std::string bar = makeBar(count, maxCount, 18);
        std::ostringstream oss;
        oss << color << std::left << std::setw(12) << label << Ansi::RESET
            << " " << color << bar << Ansi::RESET
            << " " << std::setw(3) << count << "건"
            << (flag.empty() ? "" : " " + Ansi::RED + flag + Ansi::RESET);
        std::cout << "│   " << oss.str() << "\n";
    };

    printRow("RESERVED",  summary.reservedCount,  Ansi::YELLOW);
    printRow("CONFIRMED", summary.confirmedCount, Ansi::GREEN);
    std::string pFlag = (summary.producingCount >= PRODUCING_ALERT_THRESHOLD) ? "[!]" : "";
    printRow("PRODUCING", summary.producingCount, Ansi::MAGENTA, pFlag);
    printRow("RELEASE",   summary.releaseCount,   Ansi::BLUE);
    printRow("REJECTED",  summary.rejectedCount,  Ansi::DIM);
}

void DashboardView::renderSampleStatus(const std::vector<SampleMonitorStatus>& statuses) const {
    std::cout << "│ " << Ansi::BOLD << "[ 시료별 재고 현황 ]" << Ansi::RESET << "\n";

    if (statuses.empty()) {
        std::cout << "│   등록된 시료가 없습니다.\n";
        return;
    }

    int maxQty = 1;
    for (const auto& st : statuses)
        maxQty = std::max(maxQty, st.sample.quantity);

    for (const auto& st : statuses) {
        std::string bar   = makeBar(st.sample.quantity, maxQty, 16);
        std::string color = st.isLowStock ? Ansi::RED : Ansi::GREEN;
        std::string flag  = st.isLowStock ? Ansi::RED + " [재고부족!]" + Ansi::RESET : "";

        std::ostringstream oss;
        oss << Ansi::CYAN << std::left << std::setw(5) << st.sample.id << Ansi::RESET
            << std::left << std::setw(18) << st.sample.name
            << "재고:" << color << std::right << std::setw(5) << st.sample.quantity << "개 "
            << color << bar << Ansi::RESET << flag;

        std::cout << "│   " << oss.str() << "\n";

        // 주문 수량 소계
        std::ostringstream sub;
        sub << "       예약:" << std::setw(4) << st.reservedQty
            << "  확정:" << std::setw(4) << st.confirmedQty
            << "  생산:" << std::setw(4) << st.producingQty
            << "  출고:" << std::setw(4) << st.releasedQty;
        std::cout << "│ " << Ansi::DIM << sub.str() << Ansi::RESET << "\n";
    }
}

void DashboardView::renderAlerts(const std::vector<Alert>& alerts) const {
    if (alerts.empty()) {
        std::cout << "│ " << Ansi::GREEN << "[ 알림 없음 - 시스템 정상 ]" << Ansi::RESET << "\n";
        return;
    }

    std::cout << "│ " << Ansi::BOLD << "[ 알림 (" << alerts.size() << "건) ]" << Ansi::RESET << "\n";
    for (const auto& a : alerts) {
        std::string icon, color;
        switch (a.level) {
            case Alert::Level::CRITICAL: icon = "[!!]"; color = Ansi::RED;    break;
            case Alert::Level::WARNING:  icon = "[!] "; color = Ansi::YELLOW; break;
            default:                     icon = "[i] "; color = Ansi::CYAN;   break;
        }
        std::cout << "│   " << color << icon << " " << Ansi::BOLD
                  << std::left << std::setw(8) << a.title
                  << Ansi::RESET << color << " " << a.message << Ansi::RESET << "\n";
    }
}

void DashboardView::renderFooter() const {
    printSeparator();
    std::cout << "│ ";
    std::cout << "[1]예약주문 [2]확정주문 [3]생산중 [4]시료상세 [5]전체주문 ";
    std::cout << "[S]설정 [R]새로고침 [Q]종료\n";
    std::cout << "│ " << Ansi::DIM << "자동 갱신: " << refreshSeconds_ << "초 주기" << Ansi::RESET << "\n";
    printDoubleLine();
}

// ─────────────── 상세 화면 ───────────────

void DashboardView::showOrderDetail(OrderStatus status) const {
    clearScreen();
    auto orders = controller_.getOrdersByStatus(status);
    std::string statusStr = toString(status);

    std::cout << Ansi::BOLD << Ansi::CYAN
              << "=== " << statusStr << " 주문 목록 (" << orders.size() << "건) ===\n"
              << Ansi::RESET;

    if (orders.empty()) {
        std::cout << "해당 상태의 주문이 없습니다.\n";
    } else {
        std::cout << std::left
                  << std::setw(8)  << "주문ID"
                  << std::setw(8)  << "시료ID"
                  << std::setw(18) << "고객명"
                  << std::setw(8)  << "수량"
                  << std::setw(10) << "생산예정"
                  << "주문일시\n";
        std::cout << std::string(70, '-') << "\n";
        for (const auto& o : orders) {
            std::cout << std::left
                      << std::setw(8)  << o.id
                      << std::setw(8)  << o.sampleId
                      << std::setw(18) << o.customerName
                      << std::setw(8)  << o.quantity
                      << std::setw(10) << o.requiredProduction
                      << o.orderedAt << "\n";
        }
    }
    waitEnter();
}

void DashboardView::showSampleDetail() const {
    clearScreen();
    auto samples = controller_.getAllSamples();
    auto orders  = controller_.getAllOrders();

    std::cout << Ansi::BOLD << Ansi::CYAN
              << "=== 시료별 상세 현황 (" << samples.size() << "개 시료) ===\n"
              << Ansi::RESET;

    for (const auto& s : samples) {
        std::cout << "\n" << Ansi::BOLD
                  << "[" << s.id << "] " << s.name << Ansi::RESET
                  << "  재고: " << s.quantity << "개"
                  << "  수율: " << (int)(s.yield * 100) << "%"
                  << "  사이클타임: " << s.cycleTime << "h\n";

        bool hasOrders = false;
        for (const auto& o : orders) {
            if (o.sampleId != s.id) continue;
            hasOrders = true;
            std::string color;
            switch (o.status) {
                case OrderStatus::CONFIRMED: color = Ansi::GREEN;   break;
                case OrderStatus::PRODUCING: color = Ansi::MAGENTA; break;
                case OrderStatus::RESERVED:  color = Ansi::YELLOW;  break;
                case OrderStatus::RELEASE:   color = Ansi::BLUE;    break;
                default:                     color = Ansi::DIM;     break;
            }
            std::cout << "  " << color
                      << std::left << std::setw(8) << o.id
                      << std::setw(12) << toString(o.status)
                      << std::setw(18) << o.customerName
                      << "수량: " << o.quantity
                      << Ansi::RESET << "\n";
        }
        if (!hasOrders) std::cout << "  (주문 없음)\n";
    }
    waitEnter();
}

void DashboardView::showAllOrders() const {
    clearScreen();
    auto orders = controller_.getAllOrders();

    std::cout << Ansi::BOLD << Ansi::CYAN
              << "=== 전체 주문 목록 (" << orders.size() << "건) ===\n"
              << Ansi::RESET;

    std::cout << std::left
              << std::setw(8)  << "주문ID"
              << std::setw(8)  << "시료ID"
              << std::setw(18) << "고객명"
              << std::setw(12) << "상태"
              << std::setw(8)  << "수량"
              << "주문일시\n";
    std::cout << std::string(72, '-') << "\n";

    for (const auto& o : orders) {
        std::string color;
        switch (o.status) {
            case OrderStatus::CONFIRMED: color = Ansi::GREEN;   break;
            case OrderStatus::PRODUCING: color = Ansi::MAGENTA; break;
            case OrderStatus::RESERVED:  color = Ansi::YELLOW;  break;
            case OrderStatus::RELEASE:   color = Ansi::BLUE;    break;
            case OrderStatus::REJECTED:  color = Ansi::DIM;     break;
        }
        std::cout << color
                  << std::left
                  << std::setw(8)  << o.id
                  << std::setw(8)  << o.sampleId
                  << std::setw(18) << o.customerName
                  << std::setw(12) << toString(o.status)
                  << std::setw(8)  << o.quantity
                  << o.orderedAt
                  << Ansi::RESET << "\n";
    }
    waitEnter();
}

void DashboardView::showSettings() {
    clearScreen();
    std::cout << Ansi::BOLD << Ansi::CYAN << "=== 모니터링 설정 ===\n" << Ansi::RESET;
    std::cout << "현재 자동 갱신 주기: " << refreshSeconds_ << "초\n";
    std::cout << "새 갱신 주기 입력 (5~300초, 취소: 0): ";
    int newSec = 0;
    std::cin >> newSec;
    std::cin.ignore(1000, '\n');
    if (newSec >= 5 && newSec <= 300) {
        refreshSeconds_ = newSec;
        std::cout << Ansi::GREEN << "갱신 주기가 " << refreshSeconds_ << "초로 변경되었습니다.\n" << Ansi::RESET;
    } else if (newSec != 0) {
        std::cout << Ansi::RED << "유효하지 않은 값입니다. (5~300)\n" << Ansi::RESET;
    }
    waitEnter();
}

void DashboardView::waitEnter() const {
    std::cout << "\n" << Ansi::DIM << "[ Enter ] 대시보드로 돌아가기" << Ansi::RESET;
    std::cin.ignore(1000, '\n');
}
