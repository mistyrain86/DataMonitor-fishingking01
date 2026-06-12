#include <iostream>
#include <string>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

#include "Model/SampleRepository.h"
#include "Model/OrderRepository.h"
#include "Controller/MonitorController.h"
#include "View/DashboardView.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // 데이터 경로: 기본값 "data/", 인자로 오버라이드 가능
    std::string dataDir = "data";
    if (argc >= 2) dataDir = argv[1];

    std::string sampleFile = dataDir + "/samples.json";
    std::string orderFile  = dataDir + "/orders.json";

    // 데이터 디렉토리 존재 확인
    if (!fs::exists(dataDir)) {
        std::cerr << "[경고] 데이터 디렉토리가 없습니다: " << dataDir << "\n";
        std::cerr << "       DataPersistence 시스템과 같은 경로에서 실행하거나\n";
        std::cerr << "       경로를 인자로 지정하세요: DataMonitor.exe <data경로>\n\n";
        // 빈 파일로 시작 (실시간 감시용)
    }

    SampleRepository sampleRepo(sampleFile);
    OrderRepository  orderRepo(orderFile);
    MonitorController controller(sampleRepo, orderRepo);

    std::cout << "\033[96m\033[1m";
    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║        반도체 시료 관리 - 데이터 모니터링 Tool        ║\n";
    std::cout << "║           DataPersistence 시스템 실시간 감시          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";
    std::cout << "\033[0m";
    std::cout << "데이터 경로: " << dataDir << "\n";
    std::cout << "모니터링을 시작합니다...\n\n";

    DashboardView view(controller, 5);
    view.run();

    return 0;
}
