#include "messages.hpp"

namespace protocol {
    // 大部分实现都在头文件中，这里留空

bool RTKFusionDataResponse::deserialize(const std::string& data) {
    try {
        rapidxml::xml_document<> doc;
        std::vector<char> buffer(data.begin(), data.end());
        buffer.push_back('\0');
        doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

        rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
        if (!root) return false;

        rapidxml::xml_node<>* items_node = root->first_node("Items");
        if (!items_node) return false;

        // 解析各个字段
        auto get_node_value = [&](const char* name, auto& value) {
            rapidxml::xml_node<>* node = items_node->first_node(name);
            if (node) {
                std::stringstream ss(node->value());
                ss >> value;
            }
        };

        get_node_value("Longitude", longitude);
        get_node_value("Latitude", latitude);
        get_node_value("ElpHeight", elpHeight);
        get_node_value("Yaw", yaw);

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool RTKRawDataResponse::deserialize(const std::string& data) {
    try {
        rapidxml::xml_document<> doc;
        std::vector<char> buffer(data.begin(), data.end());
        buffer.push_back('\0');
        doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

        rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
        if (!root) return false;

        rapidxml::xml_node<>* items_node = root->first_node("Items");
        if (!items_node) return false;

        // 解析各个字段
        auto get_node_value = [&](const char* name, auto& value) {
            rapidxml::xml_node<>* node = items_node->first_node(name);
            if (node) {
                std::stringstream ss(node->value());
                ss >> value;
            }
        };

        get_node_value("Longitude", longitude);
        get_node_value("Latitude", latitude);
        get_node_value("ElpHeight", elpHeight);
        get_node_value("Yaw", yaw);

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool MotionControlResponse::deserialize(const std::string& data) {
    try {
        rapidxml::xml_document<> doc;
        std::vector<char> buffer(data.begin(), data.end());
        buffer.push_back('\0');
        doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

        rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
        if (!root) return false;

        rapidxml::xml_node<>* items_node = root->first_node("Items");
        if (!items_node) return false;

        // 获取命令类型（用于判断值类型）
        int cmd = 0;
        rapidxml::xml_node<>* cmd_node = root->first_node("Command");
        if (cmd_node) {
            std::stringstream ss(cmd_node->value());
            ss >> cmd;
        }

        // 解析各个字段
        auto get_node_value = [&](const char* name, auto& value) {
            rapidxml::xml_node<>* node = items_node->first_node(name);
            if (node) {
                std::stringstream ss(node->value());
                ss >> value;
            }
        };

        // 根据命令类型决定是否解析为整数
        bool isIntValue = (cmd == 20); // 步态切换等使用整数值

        if (isIntValue) {
            int intVal = 0;
            get_node_value("Value", intVal);
            value = intVal;
        } else {
            float floatVal = 0.0f;
            get_node_value("Value", floatVal);
            value = floatVal;
        }

        get_node_value("ErrorCode", errorCode);

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}
} // namespace protocol
