#include <robotserver_sdk.h>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <future>

#include "network/asio_network_model.hpp"
#include "protocol/messages.hpp"

namespace robotserver_sdk {

// SDK版本
static const std::string SDK_VERSION = "0.1.0";

/**
 * @brief 作用域保护类
 * @tparam F 函数类型
 */
template <typename F>
class ScopeGuard {
    F f_;
public:
    ScopeGuard(F f) : f_(std::move(f)) {}
    ~ScopeGuard() { f_(); }

    // 禁止复制和移动
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&&) = delete;
    ScopeGuard& operator=(ScopeGuard&&) = delete;
};

/**
 * @brief 创建作用域保护类
 * @tparam F 函数类型
 * @param f 函数对象
 * @return 作用域保护类
 */
template <typename F>
ScopeGuard<F> makeScopeGuard(F f) {
    return ScopeGuard<F>(std::move(f));
}

/**
 * @brief 安全回调包装函数，用于捕获和处理用户回调函数中可能抛出的异常
 * @tparam Callback 回调函数类型
 * @tparam Args 回调函数参数类型
 * @param callback 用户回调函数
 * @param callbackType 回调函数类型描述，用于日志记录
 * @param args 回调函数参数
 */
template<typename Callback, typename... Args>
void safeCallback(const Callback& callback, const std::string& callbackType, Args&&... args) {
    if (!callback) {
        return;
    }

    try {
        callback(std::forward<Args>(args)...);
    } catch (const std::exception& e) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");

        std::cerr << "[" << ss.str() << "] " << callbackType << " 回调函数异常: " << e.what() << std::endl;
    } catch (...) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");

        std::cerr << "[" << ss.str() << "] " << callbackType << " 回调函数发生未知异常" << std::endl;
    }
}

RealTimeStatus convertToRealTimeStatus(const protocol::GetRealTimeStatusResponse& realTimeResp) {
    RealTimeStatus status;
    status.motionState = realTimeResp.motionState;
    status.posX = realTimeResp.posX;
    status.posY = realTimeResp.posY;
    status.posZ = realTimeResp.posZ;
    status.angleYaw = realTimeResp.angleYaw;
    status.roll = realTimeResp.roll;
    status.pitch = realTimeResp.pitch;
    status.yaw = realTimeResp.yaw;
    status.speed = realTimeResp.speed;
    status.curOdom = realTimeResp.curOdom;
    status.sumOdom = realTimeResp.sumOdom;
    status.curRuntime = realTimeResp.curRuntime;
    status.sumRuntime = realTimeResp.sumRuntime;
    status.res = realTimeResp.res;
    status.x0 = realTimeResp.x0;
    status.y0 = realTimeResp.y0;
    status.h = realTimeResp.h;
    status.electricity = realTimeResp.electricity;
    status.location = realTimeResp.location;
    status.RTKState = realTimeResp.RTKState;
    status.onDockState = realTimeResp.onDockState;
    status.gaitState = realTimeResp.gaitState;
    status.motorState = realTimeResp.motorState;
    status.chargeState = realTimeResp.chargeState;
    status.controlMode = realTimeResp.controlMode;
    status.mapUpdateState = realTimeResp.mapUpdateState;

    return status;
}

RTKFusionData convertToRTKFusionData(const protocol::RTKFusionDataResponse& rtkFusionResp) {
    RTKFusionData data;
    data.longitude = rtkFusionResp.longitude;
    data.latitude = rtkFusionResp.latitude;
    data.elpHeight = rtkFusionResp.elpHeight;
    data.yaw = rtkFusionResp.yaw;

    return data;
}

RTKRawData convertToRTKRawData(const protocol::RTKRawDataResponse& rtkRawResp) {
    RTKRawData data;
    data.longitude = rtkRawResp.longitude;
    data.latitude = rtkRawResp.latitude;
    data.elpHeight = rtkRawResp.elpHeight;
    data.yaw = rtkRawResp.yaw;

    return data;
}

// SDK实现类
class RobotServerSdkImpl : public network::INetworkCallback {
public:
    RobotServerSdkImpl(const SdkOptions& options)
        : options_(options),
          network_model_(std::make_unique<network::AsioNetworkModel>(*this)) {
        // 设置网络模型的连接超时时间
        network_model_->setConnectionTimeout(options_.connectionTimeout);
    }

    ~RobotServerSdkImpl() {
        disconnect();
    }

    bool connect(const std::string& host, uint16_t port) {
        try {
            if (isConnected()) {
                return true;
            }

            return network_model_->connect(host, port);
        } catch (const std::exception& e) {
            std::cerr << "connect 异常: " << e.what() << std::endl;
            return false;
        } catch (...) {
            std::cerr << "connect 未知异常" << std::endl;
            return false;
        }
    }
    void disconnect() {
        try {
            if (!isConnected()) {
                return;
            }

            network_model_->disconnect();
        } catch (const std::exception& e) {
            std::cerr << "disconnect 异常: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "disconnect 未知异常" << std::endl;
        }
    }

    bool isConnected() const {
        try {
            return network_model_->isConnected();
        } catch (const std::exception& e) {
            std::cerr << "isConnected 异常: " << e.what() << std::endl;
            return false;
        } catch (...) {
            std::cerr << "isConnected 未知异常" << std::endl;
            return false;
        }
    }

    RealTimeStatus request1002_RunTimeState() {
        try {
            if (!isConnected()) {
                RealTimeStatus status;
                status.errorCode = ErrorCode_RealTimeStatus::NOT_CONNECTED;
                return status;
            }

            // 创建请求消息
            protocol::GetRealTimeStatusRequest request;
            request.timestamp = getCurrentTimestamp();

            // 生成并设置序列号
            uint16_t seqNum = generateSequenceNumber();
            request.setSequenceNumber(seqNum);

            // 添加到待处理请求，标记为同步请求，并获取future
            std::future<bool> responseFuture = addPendingRequest(seqNum, protocol::MessageType::GET_REAL_TIME_STATUS_RESP);

            // 创建ScopeGuard，在函数结束时自动移除请求
            auto guard = makeScopeGuard([this, seqNum]() {
                std::lock_guard<std::mutex> lock(pending_requests_mutex_);
                pendingRequests_.erase(seqNum);
            });

            // 发送请求
            network_model_->sendMessage(request);

            // 等待响应，使用future替代条件变量
            if (responseFuture.wait_for(options_.requestTimeout) != std::future_status::ready || !responseFuture.get()) {
                RealTimeStatus status;
                status.errorCode = ErrorCode_RealTimeStatus::TIMEOUT;
                return status;
            }

            // 获取响应
            auto realTimeResp = getResponse<protocol::GetRealTimeStatusResponse>(seqNum);
            if (!realTimeResp) {
                RealTimeStatus status;
                status.errorCode = ErrorCode_RealTimeStatus::INVALID_RESPONSE;
                return status;
            }

            // 转换为SDK的RealTimeStatus
            return convertToRealTimeStatus(*realTimeResp);

        } catch (const std::exception& e) {
            std::cerr << "request1002_RunTimeState 异常: " << e.what() << std::endl;
            RealTimeStatus status;
            status.errorCode = ErrorCode_RealTimeStatus::UNKNOWN_ERROR;
            return status;
        } catch (...) {
            std::cerr << "request1002_RunTimeState 未知异常" << std::endl;
            RealTimeStatus status;
            status.errorCode = ErrorCode_RealTimeStatus::UNKNOWN_ERROR;
            return status;
        }
    }

    // 添加基于回调的异步方法实现
    void request1003_StartNavTask(const std::vector<NavigationPoint>& points, NavigationResultCallback callback) {
        try {
            if (!callback || points.empty()) {
                NavigationResult failResult;
                failResult.errorCode = ErrorCode_Navigation::INVALID_PARAM;
                safeCallback(callback, "导航结果", failResult);
                return;
            }

            if (!isConnected()) {
                NavigationResult failResult;
                failResult.errorCode = ErrorCode_Navigation::NOT_CONNECTED;
                safeCallback(callback, "导航结果", failResult);
                return;
            }

            // 创建请求消息
            protocol::NavigationTaskRequest request;
            request.timestamp = getCurrentTimestamp();

            // 生成并设置序列号
            uint16_t seqNum = generateSequenceNumber();
            request.setSequenceNumber(seqNum);

            // 转换导航点
            for (const auto& point : points) {
                protocol::NavigationPoint proto_point;
                proto_point.mapId = point.mapId;
                proto_point.value = point.value;
                proto_point.posX = point.posX;
                proto_point.posY = point.posY;
                proto_point.posZ = point.posZ;
                proto_point.angleYaw = point.angleYaw;
                proto_point.pointInfo = point.pointInfo;
                proto_point.gait = point.gait;
                proto_point.speed = point.speed;
                proto_point.manner = point.manner;
                proto_point.obsMode = point.obsMode;
                proto_point.navMode = point.navMode;
                proto_point.terrain = point.terrain;
                proto_point.posture = point.posture;
                request.points.push_back(proto_point);
            }

            // 保存回调函数
            {
                std::lock_guard<std::mutex> lock(navigation_result_callbacks_mutex_);
                navigation_result_callbacks_[seqNum] = std::move(callback);
            }

            // 发送请求
            network_model_->sendMessage(request);
        } catch (const std::exception& e) {
            std::cerr << "request1003_StartNavTask 异常: " << e.what() << std::endl;
            NavigationResult failResult;
            failResult.errorCode = ErrorCode_Navigation::UNKNOWN_ERROR;
            safeCallback(callback, "导航结果", failResult);
        } catch (...) {
            std::cerr << "request1003_StartNavTask 未知异常" << std::endl;
            NavigationResult failResult;
            failResult.errorCode = ErrorCode_Navigation::UNKNOWN_ERROR;
            safeCallback(callback, "导航结果", failResult);
        }
    }

    bool request1004_CancelNavTask() {
        try {
            if (!isConnected()) {
                return false;
            }

            // 创建请求消息
            protocol::CancelTaskRequest request;
            request.timestamp = getCurrentTimestamp();

            // 生成并设置序列号
            uint16_t seqNum = generateSequenceNumber();
            request.setSequenceNumber(seqNum);

            // 添加到待处理请求，标记为同步请求，并获取future
            std::future<bool> responseFuture = addPendingRequest(seqNum, protocol::MessageType::CANCEL_TASK_RESP);

            // 创建ScopeGuard，在函数结束时自动移除请求
            auto guard = makeScopeGuard([this, seqNum]() {
                std::lock_guard<std::mutex> lock(pending_requests_mutex_);
                pendingRequests_.erase(seqNum);
            });

            // 发送请求
            network_model_->sendMessage(request);

            // 等待响应，使用future替代条件变量
            if (responseFuture.wait_for(options_.requestTimeout) != std::future_status::ready || !responseFuture.get()) {
                return false;
            }

            // 获取响应
            auto response = getResponse<protocol::CancelTaskResponse>(seqNum);

            return response && response->errorCode == protocol::ErrorCode_CancelTask::SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "request1004_CancelNavTask 异常: " << e.what() << std::endl;
            return false;
        } catch (...) {
            std::cerr << "request1004_CancelNavTask 未知异常" << std::endl;
            return false;
        }
    }

    TaskStatusResult request1007_NavTaskState() {
        try {
            if (!isConnected()) {
                TaskStatusResult result;
                result.errorCode = ErrorCode_QueryStatus::NOT_CONNECTED;
                return result;
            }

            // 创建请求消息
            protocol::QueryStatusRequest request;
            request.timestamp = getCurrentTimestamp();

            // 生成并设置序列号
            uint16_t seqNum = generateSequenceNumber();
            request.setSequenceNumber(seqNum);

            // 添加到待处理请求，标记为同步请求，并获取future
            std::future<bool> responseFuture = addPendingRequest(seqNum, protocol::MessageType::QUERY_STATUS_RESP);

            // 创建ScopeGuard，在函数结束时自动移除请求
            auto guard = makeScopeGuard([this, seqNum]() {
                std::lock_guard<std::mutex> lock(pending_requests_mutex_);
                pendingRequests_.erase(seqNum);
            });

            // 发送请求
            network_model_->sendMessage(request);

            // 等待响应，使用future替代条件变量
            if (responseFuture.wait_for(options_.requestTimeout) != std::future_status::ready || !responseFuture.get()) {
                TaskStatusResult result;
                result.errorCode = ErrorCode_QueryStatus::TIMEOUT;
                return result;
            }

            // 获取响应
            auto queryStatusResp = getResponse<protocol::QueryStatusResponse>(seqNum);
            if (!queryStatusResp) {
                TaskStatusResult result;
                result.errorCode = ErrorCode_QueryStatus::INVALID_RESPONSE;
                return result;
            }

            // 转换为SDK的TaskStatusResult
            TaskStatusResult result;
            result.status = static_cast<Status_QueryStatus>(queryStatusResp->status);
            result.errorCode = static_cast<ErrorCode_QueryStatus>(queryStatusResp->errorCode);
            result.value = queryStatusResp->value;

            return result;

        } catch (const std::exception& e) {
            std::cerr << "request1007_NavTaskState 异常: " << e.what() << std::endl;
            TaskStatusResult result;
            result.errorCode = ErrorCode_QueryStatus::UNKNOWN_ERROR;
            return result;
        } catch (...) {
            std::cerr << "request1007_NavTaskState 未知异常" << std::endl;
            TaskStatusResult result;
            result.errorCode = ErrorCode_QueryStatus::UNKNOWN_ERROR;
            return result;
        }
    }

    RTKFusionData request2102_RTKFusionData() {
        try {
            if (!isConnected()) {
                RTKFusionData data;
                data.errorCode = ErrorCode_RTKFusion::NOT_CONNECTED;
                return data;
            }

            // 创建请求消息
            protocol::RTKFusionDataRequest request;
            request.timestamp = getCurrentTimestamp();

            // 生成并设置序列号
            uint16_t seqNum = generateSequenceNumber();
            request.setSequenceNumber(seqNum);

            // 添加到待处理请求，标记为同步请求，并获取future
            std::future<bool> responseFuture = addPendingRequest(seqNum, protocol::MessageType::RTK_FUSION_DATA_RESP);

            // 创建ScopeGuard，在函数结束时自动移除请求
            auto guard = makeScopeGuard([this, seqNum]() {
                std::lock_guard<std::mutex> lock(pending_requests_mutex_);
                pendingRequests_.erase(seqNum);
            });

            // 发送请求
            network_model_->sendMessage(request);

            // 等待响应，使用future替代条件变量
            if (responseFuture.wait_for(options_.requestTimeout) != std::future_status::ready || !responseFuture.get()) {
                RTKFusionData data;
                data.errorCode = ErrorCode_RTKFusion::TIMEOUT;
                return data;
            }

            // 获取响应
            auto rtkFusionResp = getResponse<protocol::RTKFusionDataResponse>(seqNum);
            if (!rtkFusionResp) {
                RTKFusionData data;
                data.errorCode = ErrorCode_RTKFusion::INVALID_RESPONSE;
                return data;
            }

            // 转换为SDK的RTKFusionData
            return convertToRTKFusionData(*rtkFusionResp);

        } catch (const std::exception& e) {
            std::cerr << "request2102_RTKFusionData 异常: " << e.what() << std::endl;
            RTKFusionData data;
            data.errorCode = ErrorCode_RTKFusion::UNKNOWN_ERROR;
            return data;
        } catch (...) {
            std::cerr << "request2102_RTKFusionData 未知异常" << std::endl;
            RTKFusionData data;
            data.errorCode = ErrorCode_RTKFusion::UNKNOWN_ERROR;
            return data;
        }
    }

    RTKRawData request2103_RTKRawData() {
        try {
            if (!isConnected()) {
                RTKRawData data;
                data.errorCode = ErrorCode_RTKRaw::NOT_CONNECTED;
                return data;
            }

            // 创建请求消息
            protocol::RTKRawDataRequest request;
            request.timestamp = getCurrentTimestamp();

            // 生成并设置序列号
            uint16_t seqNum = generateSequenceNumber();
            request.setSequenceNumber(seqNum);

            // 添加到待处理请求，标记为同步请求，并获取future
            std::future<bool> responseFuture = addPendingRequest(seqNum, protocol::MessageType::RTK_RAW_DATA_RESP);

            // 创建ScopeGuard，在函数结束时自动移除请求
            auto guard = makeScopeGuard([this, seqNum]() {
                std::lock_guard<std::mutex> lock(pending_requests_mutex_);
                pendingRequests_.erase(seqNum);
            });

            // 发送请求
            network_model_->sendMessage(request);

            // 等待响应，使用future替代条件变量
            if (responseFuture.wait_for(options_.requestTimeout) != std::future_status::ready || !responseFuture.get()) {
                RTKRawData data;
                data.errorCode = ErrorCode_RTKRaw::TIMEOUT;
                return data;
            }

            // 获取响应
            auto rtkRawResp = getResponse<protocol::RTKRawDataResponse>(seqNum);
            if (!rtkRawResp) {
                RTKRawData data;
                data.errorCode = ErrorCode_RTKRaw::INVALID_RESPONSE;
                return data;
            }

            // 转换为SDK的RTKRawData
            return convertToRTKRawData(*rtkRawResp);

        } catch (const std::exception& e) {
            std::cerr << "request2103_RTKRawData 异常: " << e.what() << std::endl;
            RTKRawData data;
            data.errorCode = ErrorCode_RTKRaw::UNKNOWN_ERROR;
            return data;
        } catch (...) {
            std::cerr << "request2103_RTKRawData 未知异常" << std::endl;
            RTKRawData data;
            data.errorCode = ErrorCode_RTKRaw::UNKNOWN_ERROR;
            return data;
        }
    }

    // 实现网络回调接口
    void onMessageReceived(std::unique_ptr<protocol::IMessage> message) override {
        try {
            if (!message) {
                return;
            }

            uint16_t seqNum = message->getSequenceNumber();
            protocol::MessageType msgType = message->getType();

            if (msgType == protocol::MessageType::NAVIGATION_TASK_RESP) {
                // 检查是否有等待此响应的请求
                NavigationResultCallback callback{};
                {
                    std::lock_guard<std::mutex> lock(navigation_result_callbacks_mutex_);
                    auto callbackIt = navigation_result_callbacks_.find(seqNum);
                    if (callbackIt != navigation_result_callbacks_.end()) {
                        callback = callbackIt->second;
                        navigation_result_callbacks_.erase(callbackIt);
                    }
                }

                // 如果有回调，则使用安全回调包装函数调用
                if (callback) {
                    auto* resp = dynamic_cast<protocol::NavigationTaskResponse*>(message.get());
                    if (resp) {
                        NavigationResult result;
                        result.value = resp->value;
                        result.errorCode = static_cast<ErrorCode_Navigation>(resp->errorCode);
                        result.errorStatus = static_cast<ErrorStatus_Navigation>(resp->errorStatus);
                        safeCallback(callback, "导航结果", result);
                    }
                }

                return;
            }

            // 处理其他类型的响应消息
            {
                std::lock_guard<std::mutex> lock(pending_requests_mutex_);
                auto it = pendingRequests_.find(seqNum);
                if (it != pendingRequests_.end() && it->second.expectedResponseType == msgType) {
                    it->second.response = std::move(message);
                    it->second.responseReceived = true;

                    // 使用promise通知等待线程
                    try {
                        it->second.promise->set_value(true);
                    } catch (const std::future_error&) {
                        // 忽略已经设置过值的promise
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "onMessageReceived 异常: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "onMessageReceived 未知异常" << std::endl;
        }
    }

private:

    std::future<bool> addPendingRequest(uint16_t sequenceNumber, protocol::MessageType expectedType) {
        PendingRequest req;
        req.expectedResponseType = expectedType;
        req.responseReceived = false;
        req.promise = std::make_shared<std::promise<bool>>();

        // 在插入前获取future
        std::future<bool> future = req.promise->get_future();

        {
            std::lock_guard<std::mutex> lock(pending_requests_mutex_);
            pendingRequests_[sequenceNumber] = std::move(req);
        }

        return future;
    }

    // 获取并清除响应
    template<typename ResponseType>
    std::unique_ptr<ResponseType> getResponse(uint16_t sequenceNumber) {
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        auto it = pendingRequests_.find(sequenceNumber);
        if (it == pendingRequests_.end() || !it->second.responseReceived) {
            return nullptr;
        }

        // 直接在 unique_ptr 的构造时进行 dynamic_cast
        std::unique_ptr<ResponseType> result(dynamic_cast<ResponseType*>(it->second.response.release()));

        return result;
    }

    // 获取当前时间戳
    std::string getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    SdkOptions options_;
    std::unique_ptr<network::AsioNetworkModel> network_model_;

    // 生成序列号， 从0到65535后溢出回到0
    uint16_t generateSequenceNumber() {
        static std::atomic<uint16_t> sequenceNumber = 0;
        return ++sequenceNumber;
    }

    struct PendingRequest {
        protocol::MessageType expectedResponseType{};
        std::unique_ptr<protocol::IMessage> response{};
        bool responseReceived{false};
        std::shared_ptr<std::promise<bool>> promise = std::make_shared<std::promise<bool>>();
    };

    // 使用标准的std::map和互斥锁
    std::mutex pending_requests_mutex_;
    std::map<uint16_t, PendingRequest> pendingRequests_;

    std::mutex navigation_result_callbacks_mutex_;
    std::map<uint16_t, NavigationResultCallback> navigation_result_callbacks_;
};

// RobotServerSdk类的实现
RobotServerSdk::RobotServerSdk(const SdkOptions& options)
    : impl_(std::make_unique<RobotServerSdkImpl>(options)) {
}

RobotServerSdk::~RobotServerSdk() = default;

bool RobotServerSdk::connect(const std::string& host, uint16_t port) {
    return impl_->connect(host, port);
}

void RobotServerSdk::disconnect() {
    impl_->disconnect();
}

bool RobotServerSdk::isConnected() const {
    return impl_->isConnected();
}

RealTimeStatus RobotServerSdk::request1002_RunTimeState() {
    return impl_->request1002_RunTimeState();
}

// 添加基于回调的异步方法实现
void RobotServerSdk::request1003_StartNavTask(const std::vector<NavigationPoint>& points, NavigationResultCallback callback) {
    impl_->request1003_StartNavTask(points, std::move(callback));
}

bool RobotServerSdk::request1004_CancelNavTask() {
    return impl_->request1004_CancelNavTask();
}

TaskStatusResult RobotServerSdk::request1007_NavTaskState() {
    return impl_->request1007_NavTaskState();
}

RTKFusionData RobotServerSdk::request2102_RTKFusionData() {
    return impl_->request2102_RTKFusionData();
}

RTKRawData RobotServerSdk::request2103_RTKRawData() {
    return impl_->request2103_RTKRawData();
}

std::string RobotServerSdk::getVersion() {
    return SDK_VERSION;
}

} // namespace robotserver_sdk
