#include "core\levelManager.h"
#include <fstream>

using json = nlohmann::json;

bool LevelManager::LoadLevel(const std::string& jsonPath) {
    std::ifstream file(jsonPath);
    if (!file.is_open()) return false;

    json data;
    file >> data;

    m_Colliders.clear();

    // 1. 解析 ground (地面)
    if (data.contains("ground")) {
        for (const auto& g : data["ground"]) {
            LevelCollider lc;
            lc.type = ColliderType::GROUND;
            lc.box.x = g["x"];
            lc.box.y = g["y"];
            lc.box.width = g["width"];
            lc.box.height = g["height"];
            m_Colliders.push_back(lc);
        }
    }

    // 2. 解析 pipe (水管)
    if (data.contains("pipe")) {
        for (const auto& p : data["pipe"]) {
            LevelCollider lc;
            lc.type = ColliderType::PIPE;
            lc.box.x = p["x"];
            lc.box.y = p["y"];
            lc.box.width = p["width"];
            lc.box.height = p["height"];
            lc.extraData = p["type"]; // 水管类型 (0普通, 1传送等)
            m_Colliders.push_back(lc);
        }
    }

    // 3. 解析 step (台阶)
    if (data.contains("step")) {
        for (const auto& s : data["step"]) {
            LevelCollider lc;
            lc.type = ColliderType::STEP;
            lc.box.x = s["x"];
            lc.box.y = s["y"];
            lc.box.width = s["width"];
            lc.box.height = s["height"];
            m_Colliders.push_back(lc);
        }
    }

    // 4. 解析 brick (砖块)
    if (data.contains("brick")) {
        for (const auto& b : data["brick"]) {
            LevelCollider lc;
            lc.type = ColliderType::BRICK;
            lc.box.x = b["x"];
            lc.box.y = b["y"];
            lc.box.width = 16; // JSON里没写宽高，默认16x16
            lc.box.height = 16;
            lc.extraData = b["type"]; // 砖块类型
            m_Colliders.push_back(lc);
        }
    }

    // 5. 解析 box (问号块/奖励块)
    if (data.contains("box")) {
        for (const auto& b : data["box"]) {
            LevelCollider lc;
            lc.type = ColliderType::BOX;
            lc.box.x = b["x"];
            lc.box.y = b["y"];
            lc.box.width = 16; // JSON里没写宽高，默认16x16
            lc.box.height = 16;
            lc.extraData = b["type"]; // 箱子类型 (1蘑菇, 3星星, 4金币等)
            m_Colliders.push_back(lc);
        }
    }

    return true;
}