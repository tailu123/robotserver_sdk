#include <cstdint>
#include <robotserver_sdk.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <robotserver_sdk.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <filesystem>
#include <fstream>

std::vector<robotserver_sdk::NavigationPoint> loadDefaultNavigationPoints(const std::string& configPath) {
    std::vector<robotserver_sdk::NavigationPoint> points;
    try {
        // 检查文件是否存在
        if (!std::filesystem::exists(configPath)) {
            std::cerr << "配置文件不存在: " << configPath << std::endl;
            return points;
        }

        // 读取JSON文件
        std::ifstream file(configPath);
        nlohmann::json jsonArray;
        file >> jsonArray;

        // 解析每个导航点
        for (const auto& jsonPoint : jsonArray) {
            points.push_back(robotserver_sdk::NavigationPoint::fromJson(jsonPoint));
        }

        std::cout << "成功从配置文件加载了 " << points.size() << " 个导航点" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "加载配置文件失败: " << e.what() << std::endl;
    }
    return points;
}

// 辅助函数：加载默认导航点
std::vector<robotserver_sdk::NavigationPoint> loadNavigationPoints() {
    // 尝试多个可能的路径
    std::vector<std::filesystem::path> possiblePaths;

    try {
        // 1. 尝试相对于可执行文件的路径
        std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe");
        std::filesystem::path exeDir = exePath.parent_path();

        possiblePaths.push_back(exeDir / "default_navigation_points.json");                  // 与可执行文件同目录
        possiblePaths.push_back(exeDir / "basic" / "default_navigation_points.json");        // 可执行文件的 basic 子目录
        possiblePaths.push_back(exeDir.parent_path() / "examples" / "basic" / "default_navigation_points.json"); // bin/../examples/basic/

        // 2. 尝试相对于当前工作目录的路径
        std::filesystem::path currentDir = std::filesystem::current_path();
        possiblePaths.push_back(currentDir / "default_navigation_points.json");
        possiblePaths.push_back(currentDir / "examples" / "basic" / "default_navigation_points.json");

        // 3. 尝试源代码目录的路径（假设在构建目录中运行）
        possiblePaths.push_back(currentDir.parent_path() / "examples" / "basic" / "default_navigation_points.json");
    }
    catch (const std::exception& e) {
        std::cerr << "获取可执行文件路径时出错: " << e.what() << std::endl;
    }

    // 尝试每个可能的路径
    for (const auto& path : possiblePaths) {
        std::cout << "尝试加载配置文件: " << path.string() << std::endl;
        if (std::filesystem::exists(path)) {
            std::vector<robotserver_sdk::NavigationPoint> points = loadDefaultNavigationPoints(path.string());
            if (!points.empty()) {
                return points;
            }
        }
    }

    // 如果所有路径都失败，尝试使用硬编码的路径
    std::cerr << "无法找到配置文件，尝试使用硬编码路径" << std::endl;
    return loadDefaultNavigationPoints("./default_navigation_points.json");
}

std::vector<robotserver_sdk::NavigationPoint> g_points = loadNavigationPoints();


int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc < 3) {
        std::cerr << "用法: " << argv[0] << " <主机地址> <端口>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

    std::cout << "机器狗 RobotServer SDK 示例程序" << std::endl;
    std::cout << "SDK 版本: " << robotserver_sdk::RobotServerSdk::getVersion() << std::endl;
    std::cout << "连接到: " << host << ":" << port << std::endl;

    try {
        // 创建 SDK 实例
        robotserver_sdk::SdkOptions options;
        options.connectionTimeout = std::chrono::milliseconds(5000);
        options.requestTimeout = std::chrono::milliseconds(3000);

        robotserver_sdk::RobotServerSdk sdk(options);

        // 连接到机器狗控制系统
        if (!sdk.connect(host, port)) {
            std::cerr << "连接失败!" << std::endl;
            return 1;
        }

        std::cout << "连接成功!" << std::endl;

        std::vector<robotserver_sdk::NavigationPoint> points = g_points;

        // 标记是否收到导航响应
        std::atomic<bool> navigationResponseReceived{false};

        // 异步发送导航任务（使用回调形式）
        std::cout << "开始导航任务..." << std::endl;
        sdk.request1003_StartNavTask(points, [&](const robotserver_sdk::NavigationResult&) {
            // 标记已收到导航响应
            navigationResponseReceived = true;
        });

        while (not navigationResponseReceived) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "开始导航任务..." << std::endl;
        navigationResponseReceived = false;
        sdk.request1003_StartNavTask(points, [&](const robotserver_sdk::NavigationResult&) {
            // 标记已收到导航响应
            navigationResponseReceived = true;
        });

        while (not navigationResponseReceived) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 断开连接
        sdk.disconnect();
        std::cout << "已断开连接" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
