#include <robotserver_sdk.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstdio>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iomanip>
#include <sstream>

using namespace robotserver_sdk;

// 全局变量，用于控制程序执行
std::atomic<bool> running(true);
std::atomic<char> current_key(0);

// 获取当前时间字符串，用于日志记录
std::string getCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%H:%M:%S");
    return ss.str();
}

// 设置终端为非阻塞模式，使得可以不按回车键即可读取按键
void setNonBlockingMode() {
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag &= ~(ICANON | ECHO);
    ttystate.c_cc[VMIN] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

    // 设置为非阻塞模式
    int flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

// 恢复终端设置
void resetTerminalMode() {
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

    // 恢复阻塞模式
    int flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}

// 打印按键说明
void printKeyInstructions() {
    std::cout << "\n======== 按键控制说明 ========" << std::endl;
    std::cout << "w - 前进(0.5 m/s)" << std::endl;
    std::cout << "s - 后退(0.3 m/s)" << std::endl;
    std::cout << "a - 左移(0.1 m/s)" << std::endl;
    std::cout << "d - 右移(0.1 m/s)" << std::endl;
    std::cout << "j - 左转(0.3 rad/s)" << std::endl;
    std::cout << "k - 右转(0.3 rad/s)" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "1 - 切换到站立高度" << std::endl;
    std::cout << "2 - 切换到匍匐高度" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "3 - 切换到行走步态" << std::endl;
    std::cout << "4 - 切换到普通梯步步态" << std::endl;
    std::cout << "5 - 切换到斜坡/防滑步态" << std::endl;
    std::cout << "6 - 切换到感知梯步步态" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "空格键 - 停止" << std::endl;
    std::cout << "q - 退出程序" << std::endl;
    std::cout << "h - 显示这个帮助信息" << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "松开方向键将自动停止" << std::endl;
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
            std::cout << "命令过于频繁" << std::endl;
            break;
        case ErrorCode_MotionControl::UNKNOWN_ERROR:
            std::cout << "未知错误" << std::endl;
            break;
        default:
            std::cout << "未知错误码: " << static_cast<int>(errorCode) << std::endl;
            break;
    }
}

// 执行移动命令
void executeMove(RobotServerSdk& sdk, SpeedCommand cmd, float speed) {
    std::cout << "[" << getCurrentTimeString() << "] 执行速度命令... ";
    auto result = sdk.request2_SpeedControl(cmd, speed);
    std::cout << "结果: ";
    printErrorCodeMessage(result.errorCode);
}

// 执行停止命令
void executeStop(RobotServerSdk& sdk) {
    std::cout << "[" << getCurrentTimeString() << "] 执行停止命令... ";
    auto result = sdk.request2_ActionControl(ActionCommand::STOP);
    std::cout << "结果: ";
    printErrorCodeMessage(result.errorCode);
}

// 键盘读取线程
void keyboardThread() {
    char c;
    while (running) {
        if (read(STDIN_FILENO, &c, 1) > 0) {
            current_key.store(c);

            // 如果按下q，退出程序
            if (c == 'q') {
                running.store(false);
                break;
            }
        } else {
            // 没有按键输入时，将当前键值设为0
            current_key.store(0);
        }

        // 短暂休眠，减少CPU占用
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// 实现按键控制逻辑
void controlLoop(RobotServerSdk& sdk) {
    char last_key = 0;
    bool is_moving = false;

    std::cout << "开始控制循环，按 'h' 查看按键说明" << std::endl;

    while (running) {
        char current = current_key.load();

        // 当前键与上一次不同，处理按键
        if (current != last_key) {
            // 如果按下了有效的控制键
            if (current == 'w' || current == 's' || current == 'a' ||
                current == 'd' || current == 'j' || current == 'k') {

                std::cout << "\n按下键: " << current << std::endl;

                switch (current) {
                    case 'w': // 前进
                        executeMove(sdk, SpeedCommand::FORWARD, 0.5f);
                        break;
                    case 's': // 后退
                        executeMove(sdk, SpeedCommand::BACKWARD, 0.3f);
                        break;
                    case 'a': // 左移
                        executeMove(sdk, SpeedCommand::TRANSVERSE_LEFT, 0.1f);
                        break;
                    case 'd': // 右移
                        executeMove(sdk, SpeedCommand::TRANSVERSE_RIGHT, 0.1f);
                        break;
                    case 'j': // 左转
                        executeMove(sdk, SpeedCommand::TURN_LEFT, 0.3f);
                        break;
                    case 'k': // 右转
                        executeMove(sdk, SpeedCommand::TURN_RIGHT, 0.3f);
                        break;
                }

                is_moving = true;
            }
            // 如果松开了控制键（当前无按键且之前在移动）
            else if (current == 0 && is_moving) {
                std::cout << "\n松开方向键，停止移动" << std::endl;
                executeStop(sdk);
                is_moving = false;
            }
            // 空格键也可以停止
            else if (current == ' ' && is_moving) {
                std::cout << "\n按下空格键，停止移动" << std::endl;
                executeStop(sdk);
                is_moving = false;
            }
            // 显示帮助信息
            else if (current == 'h') {
                printKeyInstructions();
            }
            // 新按键处理
            else if (current == '1') {
                std::cout << "\n切换到站立高度" << std::endl;
                auto result = sdk.request2_SwitchBodyHeight(0);
                std::cout << "结果: ";
                printErrorCodeMessage(result.errorCode);
            }
            else if (current == '2') {
                std::cout << "\n切换到匍匐高度" << std::endl;
                auto result = sdk.request2_SwitchBodyHeight(1);
                std::cout << "结果: ";
                printErrorCodeMessage(result.errorCode);
            }
            else if (current == '3') {
                std::cout << "\n切换到行走步态" << std::endl;
                auto result = sdk.request2_SwitchGait(GaitMode::WALKING);
                std::cout << "结果: ";
                printErrorCodeMessage(result.errorCode);
            }
            else if (current == '4') {
                std::cout << "\n切换到普通梯步步态" << std::endl;
                auto result = sdk.request2_SwitchGait(GaitMode::NORMAL_STEPPING);
                std::cout << "结果: ";
                printErrorCodeMessage(result.errorCode);
            }
            else if (current == '5') {
                std::cout << "\n切换到斜坡/防滑步态" << std::endl;
                auto result = sdk.request2_SwitchGait(GaitMode::SLOPE_ANTI_SLIP);
                std::cout << "结果: ";
                printErrorCodeMessage(result.errorCode);
            }
            else if (current == '6') {
                std::cout << "\n切换到感知梯步步态" << std::endl;
                auto result = sdk.request2_SwitchGait(GaitMode::SENSING_STEPPING);
                std::cout << "结果: ";
                printErrorCodeMessage(result.errorCode);
            }

            // 更新上一次的按键
            last_key = current;
        }

        // 短暂休眠，减少CPU占用
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

    // 连接到服务器
    if (!sdk.connect(host, port)) {
        std::cerr << "连接失败" << std::endl;
        return 1;
    }

    std::cout << "连接成功，SDK版本: " << sdk.getVersion() << std::endl;

    try {
        // 设置终端为非阻塞模式
        setNonBlockingMode();

        // 打印按键说明
        printKeyInstructions();

        // 启动键盘读取线程
        std::thread keyboard_thread(keyboardThread);

        // 运行控制循环
        controlLoop(sdk);

        // 等待键盘线程结束
        if (keyboard_thread.joinable()) {
            keyboard_thread.join();
        }

        // 确保最后停止运动
        std::cout << "\n执行最终停止命令..." << std::endl;
        sdk.request2_ActionControl(ActionCommand::STOP);

        // 恢复终端设置
        resetTerminalMode();

    } catch (const std::exception& e) {
        // 恢复终端设置
        resetTerminalMode();
        std::cerr << "程序执行过程中出现异常: " << e.what() << std::endl;
    }

    // 断开连接
    sdk.disconnect();
    std::cout << "断开连接" << std::endl;

    return 0;
}
