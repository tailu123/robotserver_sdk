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
} // namespace protocol
