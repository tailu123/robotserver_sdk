#include <robotserver_sdk.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <iomanip>

using namespace robotserver_sdk;

// 打印错误码提示信息
void printErrorCodeMessage(ErrorCode_MotionControl errorCode) {
    switch (errorCode) {
        case ErrorCode_MotionControl::SUCCESS:
            std::cout << "成功" << std::endl;
            break;
        case ErrorCode_MotionControl::FAILURE:
            std::cout << "失败" << std::endl;
            break;
        case ErrorCode_MotionControl::NOT_CONNECTED:
            std::cout << "未连接" << std::endl;
            break;
        case ErrorCode_MotionControl::TIMEOUT:
            std::cout << "超时" << std::endl;
            break;
        case ErrorCode_MotionControl::TOO_FREQUENT:
            std::cout << "命令发送过于频繁，请稍后再试" << std::endl;
            break;
        case ErrorCode_MotionControl::UNKNOWN_ERROR:
            std::cout << "未知错误" << std::endl;
            break;
        default:
            std::cout << "未知错误码: " << static_cast<int>(errorCode) << std::endl;
            break;
    }
}

// 测试速度控制命令
void testSpeedControl(RobotServerSdk& sdk) {
    std::cout << "\n===== 测试速度控制命令 =====" << std::endl;

    // 测试前进
    std::cout << "设置前进速度 0.5 m/s..." << std::endl;
    auto result = sdk.request2_SpeedControl(SpeedCommand::FORWARD, 0.5f);
    std::cout << "结果: ";
    printErrorCodeMessage(result.errorCode);

    // 等待一段时间
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 测试左转
    std::cout << "设置左转速度 0.3 rad/s..." << std::endl;
    result = sdk.request2_SpeedControl(SpeedCommand::TURN_LEFT, 0.3f);
    std::cout << "结果: ";
    printErrorCodeMessage(result.errorCode);

    // 等待一段时间
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 测试停止
    std::cout << "停止..." << std::endl;
    auto stopResult = sdk.request2_ActionControl(ActionCommand::STOP);
    std::cout << "结果: ";
    printErrorCodeMessage(stopResult.errorCode);
}

// 测试动作控制命令
void testActionControl(RobotServerSdk& sdk) {
    std::cout << "\n===== 测试动作控制命令 =====" << std::endl;

    // 测试站立
    std::cout << "执行站立动作..." << std::endl;
    auto result = sdk.request2_ActionControl(ActionCommand::STAND_UP);
    std::cout << "结果: ";
    printErrorCodeMessage(result.errorCode);

    // 等待一段时间
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 测试力控
    std::cout << "切换力控模式..." << std::endl;
    result = sdk.request2_ActionControl(ActionCommand::FORCE);
    std::cout << "结果: ";
    printErrorCodeMessage(result.errorCode);

    // 等待一段时间
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

// 测试配置命令
void testConfigCommands(RobotServerSdk& sdk) {
    std::cout << "\n===== 测试配置命令 =====" << std::endl;

    // 测试切换步态 (原来用request2_SetGaitMode，现在用request2_SwitchGait)
    std::cout << "切换到普通梯步步态..." << std::endl;
    auto result = sdk.request2_SwitchGait(GaitMode::NORMAL_STEPPING);
    std::cout << "结果: ";
    printErrorCodeMessage(result.errorCode);

    // 等待一段时间
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 测试切换身体高度
    std::cout << "切换身体高度到匍匐模式..." << std::endl;
    result = sdk.request2_SwitchBodyHeight(1);
    std::cout << "结果: ";
    printErrorCodeMessage(result.errorCode);

    // 等待一段时间
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 切换回站立模式
    std::cout << "切换身体高度到站立模式..." << std::endl;
    result = sdk.request2_SwitchBodyHeight(0);
    std::cout << "结果: ";
    printErrorCodeMessage(result.errorCode);
}

// 测试频率限制
void testFrequencyLimit(RobotServerSdk& sdk) {
    std::cout << "\n===== 测试速度命令频率限制 =====" << std::endl;

    std::cout << "连续发送前进命令..." << std::endl;

    // 第一次发送应该成功
    auto result1 = sdk.request2_SpeedControl(SpeedCommand::FORWARD, 0.3f);
    std::cout << "第1次发送结果: ";
    printErrorCodeMessage(result1.errorCode);

    // 立即发送第二次，应该失败（TOO_FREQUENT）
    auto result2 = sdk.request2_SpeedControl(SpeedCommand::FORWARD, 0.4f);
    std::cout << "第2次发送结果: ";
    printErrorCodeMessage(result2.errorCode);

    // 等待300ms后发送第三次，应该成功
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    auto result3 = sdk.request2_SpeedControl(SpeedCommand::FORWARD, 0.5f);
    std::cout << "等待300ms后发送，结果: ";
    printErrorCodeMessage(result3.errorCode);

    // 最后停止
    std::this_thread::sleep_for(std::chrono::seconds(1));
    sdk.request2_ActionControl(ActionCommand::STOP);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "用法: " << argv[0] << " <主机地址> <端口>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

    std::cout << "连接到 " << host << ":" << port << std::endl;

    // 创建SDK实例
    SdkOptions options;
    RobotServerSdk sdk(options);

    // 连接到服务器
    if (!sdk.connect(host, port)) {
        std::cerr << "连接失败" << std::endl;
        return 1;
    }

    std::cout << "连接成功，SDK版本: " << sdk.getVersion() << std::endl;

    try {
        // 运行测试
        testSpeedControl(sdk);
        testActionControl(sdk);
        testConfigCommands(sdk);
        testFrequencyLimit(sdk);

        // 确保最后停止运动
        std::cout << "\n执行最终停止命令..." << std::endl;
        sdk.request2_ActionControl(ActionCommand::STOP);

    } catch (const std::exception& e) {
        std::cerr << "测试过程中出现异常: " << e.what() << std::endl;
    }

    // 断开连接
    sdk.disconnect();
    std::cout << "断开连接" << std::endl;

    return 0;
}
