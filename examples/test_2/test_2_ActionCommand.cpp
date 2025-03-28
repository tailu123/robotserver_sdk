#include <robotserver_sdk.h>
#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace robotserver_sdk;

// 获取当前时间字符串，用于日志记录
std::string getCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%H:%M:%S");
    return ss.str();
}

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

// 创建命令映射，用于将用户输入转换为ActionCommand
std::map<std::string, ActionCommand> createCommandMap() {
    std::map<std::string, ActionCommand> commandMap;
    commandMap["over"] = ActionCommand::MOTION_CONTROL_OVER;      // 运动控制结束
    commandMap["emergency"] = ActionCommand::SOFT_EMERGENCY_STOP; // 软急停
    commandMap["stop"] = ActionCommand::STOP;                     // 停止/站住
    commandMap["down"] = ActionCommand::FINISH;                   // 完成/趴下
    commandMap["up"] = ActionCommand::STAND_UP;                   // 站立/站起
    commandMap["force"] = ActionCommand::FORCE;                   // 力控
    commandMap["step"] = ActionCommand::START_STEPPING;           // 开始踏步
    commandMap["charge"] = ActionCommand::GO_CHARGE;              // 前往充电
    commandMap["exit"] = ActionCommand::EXIT_CHARGE;              // 退出充电
    return commandMap;
}

// 打印命令帮助信息
void printHelp(const std::map<std::string, ActionCommand>&) {
    std::cout << "\n可用命令列表：" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "over      - 运动控制结束" << std::endl;
    std::cout << "emergency - 软急停" << std::endl;
    std::cout << "stop      - 停止/站住" << std::endl;
    std::cout << "down      - 完成/趴下" << std::endl;
    std::cout << "up        - 站立/站起" << std::endl;
    std::cout << "force     - 力控" << std::endl;
    std::cout << "step      - 开始踏步" << std::endl;
    std::cout << "charge    - 前往充电" << std::endl;
    std::cout << "exit      - 退出充电" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "w         - 前进（0.5m/s，持续2秒）" << std::endl;
    std::cout << "s         - 后退（0.3m/s，持续2秒）" << std::endl;
    std::cout << "a         - 左移（0.1m/s，持续2秒）" << std::endl;
    std::cout << "d         - 右移（0.1m/s，持续2秒）" << std::endl;
    std::cout << "j         - 左转（0.3rad/s，持续2秒）" << std::endl;
    std::cout << "k         - 右转（0.3rad/s，持续2秒）" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "status    - 查询实时状态" << std::endl;
    std::cout << "help      - 显示此帮助信息" << std::endl;
    std::cout << "quit      - 退出程序" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "height0   - 切换到站立高度" << std::endl;
    std::cout << "height1   - 切换到匍匐高度" << std::endl;
    std::cout << "gait0     - 切换到行走步态" << std::endl;
    std::cout << "gait1     - 切换到普通梯步步态" << std::endl;
    std::cout << "gait2     - 切换到斜坡/防滑步态" << std::endl;
    std::cout << "gait4     - 切换到感知梯步步态" << std::endl;
    std::cout << "------------------------------" << std::endl;
}

// 执行动作命令
void executeAction(RobotServerSdk& sdk, ActionCommand cmd) {
    std::cout << "[" << getCurrentTimeString() << "] 执行动作命令... ";
    auto result = sdk.request2_ActionControl(cmd);
    std::cout << "结果: ";
    printErrorCodeMessage(result.errorCode);
}

// 执行速度命令（带自动停止）
void executeSpeedCommand(RobotServerSdk& sdk, SpeedCommand cmd, float speed, int durationMs) {
    std::cout << "[" << getCurrentTimeString() << "] 执行速度命令... ";
    sdk.request2_SpeedControl(cmd, speed);
}

// 查询实时状态
void queryStatus(RobotServerSdk& sdk) {
    std::cout << "[" << getCurrentTimeString() << "] 查询实时状态..." << std::endl;
    auto status = sdk.request1002_RunTimeState();

    if (status.errorCode == ErrorCode_RealTimeStatus::SUCCESS) {
        std::cout << "位置: (" << status.posX << ", " << status.posY << ", " << status.posZ << ")" << std::endl;
        std::cout << "角度: 偏航=" << status.yaw << ", 横滚=" << status.roll << ", 俯仰=" << status.pitch << std::endl;
        std::cout << "电量: " << status.electricity << "%" << std::endl;
        std::cout << "运动状态: " << status.motionState << std::endl;
        std::cout << "步态状态: " << status.gaitState << std::endl;
    } else {
        std::cout << "查询失败，错误码: " << static_cast<int>(status.errorCode) << std::endl;
    }
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

    // 创建命令映射
    auto commandMap = createCommandMap();

    // 连接到服务器
    if (!sdk.connect(host, port)) {
        std::cerr << "连接失败" << std::endl;
        return 1;
    }

    std::cout << "连接成功，SDK版本: " << sdk.getVersion() << std::endl;
    std::cout << "输入 'help' 查看可用命令" << std::endl;

    try {
        std::string input;
        bool running = true;

        while (running) {
            std::cout << "\n> ";
            std::getline(std::cin, input);

            // 处理用户输入
            if (input == "quit") {
                running = false;
            }
            else if (input == "help") {
                printHelp(commandMap);
            }
            else if (input == "status") {
                queryStatus(sdk);
            }
            // 使用单键控制移动和转向
            else if (input == "w") {
                executeSpeedCommand(sdk, SpeedCommand::FORWARD, 0.5f, 2000);
            }
            else if (input == "s") {
                executeSpeedCommand(sdk, SpeedCommand::BACKWARD, 0.3f, 2000);
            }
            else if (input == "a") {
                executeSpeedCommand(sdk, SpeedCommand::TRANSVERSE_LEFT, 0.1f, 2000);
            }
            else if (input == "d") {
                executeSpeedCommand(sdk, SpeedCommand::TRANSVERSE_RIGHT, 0.1f, 2000);
            }
            else if (input == "j") {
                executeSpeedCommand(sdk, SpeedCommand::TURN_LEFT, 0.3f, 2000);
            }
            else if (input == "k") {
                executeSpeedCommand(sdk, SpeedCommand::TURN_RIGHT, 0.3f, 2000);
            }
            else if (input == "height0") {
                std::cout << "[" << getCurrentTimeString() << "] 切换到站立高度... ";
                auto result = sdk.request2_SwitchBodyHeight(0);
                std::cout << "结果: ";
                printErrorCodeMessage(result.errorCode);
            }
            else if (input == "height1") {
                std::cout << "[" << getCurrentTimeString() << "] 切换到匍匐高度... ";
                auto result = sdk.request2_SwitchBodyHeight(1);
                std::cout << "结果: ";
                printErrorCodeMessage(result.errorCode);
            }
            else if (input == "gait0") {
                std::cout << "[" << getCurrentTimeString() << "] 切换到行走步态... ";
                auto result = sdk.request2_SwitchGait(GaitMode::WALKING);
                std::cout << "结果: ";
                printErrorCodeMessage(result.errorCode);
            }
            else if (input == "gait1") {
                std::cout << "[" << getCurrentTimeString() << "] 切换到普通梯步步态... ";
                auto result = sdk.request2_SwitchGait(GaitMode::NORMAL_STEPPING);
                std::cout << "结果: ";
                printErrorCodeMessage(result.errorCode);
            }
            else if (input == "gait2") {
                std::cout << "[" << getCurrentTimeString() << "] 切换到斜坡/防滑步态... ";
                auto result = sdk.request2_SwitchGait(GaitMode::SLOPE_ANTI_SLIP);
                std::cout << "结果: ";
                printErrorCodeMessage(result.errorCode);
            }
            else if (input == "gait4") {
                std::cout << "[" << getCurrentTimeString() << "] 切换到感知梯步步态... ";
                auto result = sdk.request2_SwitchGait(GaitMode::SENSING_STEPPING);
                std::cout << "结果: ";
                printErrorCodeMessage(result.errorCode);
            }
            else if (commandMap.find(input) != commandMap.end()) {
                // 执行动作命令
                executeAction(sdk, commandMap[input]);
            }
            else {
                std::cout << "未知命令: " << input << std::endl;
                std::cout << "输入 'help' 查看可用命令" << std::endl;
            }
        }

        // 确保最后停止运动
        std::cout << "\n执行最终停止命令..." << std::endl;
        sdk.request2_ActionControl(ActionCommand::STOP);

    } catch (const std::exception& e) {
        std::cerr << "程序执行过程中出现异常: " << e.what() << std::endl;
    }

    // 断开连接
    sdk.disconnect();
    std::cout << "断开连接" << std::endl;

    return 0;
}
