#pragma once

#include "types.h"
#include <memory>
#include <string>
#include <future>

namespace robotserver_sdk {

// 前向声明，隐藏实现细节
class RobotServerSdkImpl;

/**
 * @brief robotserver sdk主类
 *
 * 该类提供了与机器狗进行通信的主要接口，包括连接管理、
 * 导航任务控制、状态查询等功能。
 */
class RobotServerSdk {
public:
    /**
     * @brief 构造函数
     * @param options SDK配置选项
     */
    explicit RobotServerSdk(const SdkOptions& options = SdkOptions());

    /**
     * @brief 析构函数
     */
    ~RobotServerSdk();

    /**
     * @brief 禁用拷贝构造函数
     */
    RobotServerSdk(const RobotServerSdk&) = delete;

    /**
     * @brief 禁用赋值操作符
     */
    RobotServerSdk& operator=(const RobotServerSdk&) = delete;

    /**
     * @brief 连接到机器狗控制系统
     * @param host 主机地址
     * @param port 端口号
     * @return 连接是否成功
     */
    bool connect(const std::string& host, uint16_t port);

    /**
     * @brief 断开与机器狗控制系统的连接
     */
    void disconnect();

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief request1002 获取机器狗的实时状态
     * @return 实时状态信息
     */
    RealTimeStatus request1002_RunTimeState();

    /**
     * @brief request1003 基于回调的异步开始导航任务
     * @param points 导航点列表
     * @param callback 导航结果回调函数
     */
    void request1003_StartNavTask(const std::vector<NavigationPoint>& points, NavigationResultCallback callback);

    /**
     * @brief request1004 取消当前导航任务
     * @return 操作是否成功
     */
    bool request1004_CancelNavTask();

    /**
     * @brief request1007 查询当前导航任务状态
     * @return 任务状态查询结果
     */
    TaskStatusResult request1007_NavTaskState();

    /**
     * @brief request2102 获取RTK融合数据
     * @return RTK融合数据
     */
    RTKFusionData request2102_RTKFusionData();

    /**
     * @brief request2103 获取RTK原始数据
     * @return RTK原始数据
     */
    RTKRawData request2103_RTKRawData();

    /**
     * @brief 获取SDK版本
     * @return SDK版本字符串
     */
    static std::string getVersion();

    /**
     * @brief request2_SpeedControl 设置速度控制指令
     * @param cmd 速度命令类型
     * @param speed 速度值（m/s或rad/s）
     * @return 操作结果
     *
     * 注意：根据协议要求，速度控制指令的发送频率不应超过5Hz。
     * 如果调用过于频繁（小于200ms间隔），函数将返回失败结果。
     */
    MotionControlResult request2_SpeedControl(SpeedCommand cmd, float speed);

    /**
     * @brief request2_ActionControl 执行动作控制指令
     * @param cmd 动作命令类型
     * @return 操作结果
     */
    MotionControlResult request2_ActionControl(ActionCommand cmd);

    /**
     * @brief request2_Configure 设置配置参数
     * @param cmd 配置命令类型
     * @param value 配置值
     * @return 操作结果
     */
    MotionControlResult request2_Configure(ConfigCommand cmd, int value);

    /**
     * @brief request2_SwitchBodyHeight 切换身体高度
     * @param height 身体高度模式：0表示站立，1表示匍匐
     * @return 操作结果
     */
    MotionControlResult request2_SwitchBodyHeight(int height);

    /**
     * @brief request2_SwitchGait 切换步态模式
     * @param mode 步态模式
     * @return 操作结果
     */
    MotionControlResult request2_SwitchGait(GaitMode mode);

private:
    std::unique_ptr<RobotServerSdkImpl> impl_; ///< PIMPL实现
};

} // namespace robotserver_sdk
