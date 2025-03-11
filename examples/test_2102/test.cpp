#include <cstdint>
#include <robotserver_sdk.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <filesystem>
#include <fstream>


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

        // 等待2秒
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // 获取RTK融合数据
        robotserver_sdk::RTKFusionData rtkFusionData = sdk.request2102_RTKFusionData();
        std::cout << "RTK融合数据: " << rtkFusionData.longitude << ", " << rtkFusionData.latitude << ", " << rtkFusionData.altitude << std::endl;

        // 获取RTK原始数据
        robotserver_sdk::RTKRawData rtkRawData = sdk.request2103_RTKRawData();
        std::cout << "RTK原始数据: " << rtkRawData.serialNo << ", " << rtkRawData.utc << ", " << rtkRawData.lat << ", " << rtkRawData.lon << ", " << rtkRawData.elpHeight << std::endl;

        // 断开连接
        sdk.disconnect();
        std::cout << "已断开连接" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
