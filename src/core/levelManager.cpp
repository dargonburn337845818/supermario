#include "core\LevelManager.h"
#include "core\Game.h"
#include "Utils\json.hpp"
#include <fstream>
#include <iostream>
#include <graphics.h>

using json = nlohmann::json;

LevelManager::LevelManager() {}

void LevelManager::OnBlockHit(int index, SolidType type) {
    if (index < 0 || index >= m_SolidColliders.size()) return;
    
    auto& block = m_SolidColliders[index];
    // 处理问号箱
    if (type == SolidType::BOX && !block.isHit) {
        block.isHit = true; // 标记为已撞击
        block.bounceTimer = 10;
        // 弹出金币特效
        CoinEffect coin;
        coin.x = block.bounds.x + block.bounds.width / 2 - 16; // 居中
        coin.y = block.bounds.y - 32;                          // 从顶部弹出
        m_CoinEffects.push_back(coin);
        
        // 这里还可以加：播放音效、加分等逻辑
    }
    // 处理砖块 (小马里奥顶不动，大马里奥顶碎，目前先留空，或者做个弹起动画)
    else if (type == SolidType::BRICK) {
        block.bounceTimer = 10; // 砖块也能弹起
    }
}

void LevelManager::SpawnCoinEffect(int blockIndex) {
    if (blockIndex < 0 || blockIndex >= m_SolidColliders.size()) return;
    
    auto& block = m_SolidColliders[blockIndex];
    // 金币出现的X位置：方块水平居中；Y位置：方块顶部
    CoinEffect coin;
    coin.x = block.bounds.x + block.bounds.width / 2 - 16; // 假设金币宽32，居中偏移16
    coin.y = block.bounds.y - 32; // 从方块顶部弹出
    
    m_CoinEffects.push_back(coin);
}

void LevelManager::UpdateEffects() {
    for (auto& coin : m_CoinEffects) {
        coin.velY += 0.4f; // 金币受重力影响
        coin.y += coin.velY;
        coin.life--;
    }
    // 清除生命结束的金币
    m_CoinEffects.erase(std::remove_if(m_CoinEffects.begin(), m_CoinEffects.end(),
        [](const CoinEffect& c) { return c.life <= 0; }), m_CoinEffects.end());

    for (auto& block : m_SolidColliders) {
        if (block.bounceTimer > 0) {
            block.bounceTimer--;
            // 前5帧上升，后5帧下落，绝对精准归零！
            if (block.bounceTimer > 5) {
                block.bounceOffset = -4.0f; // 上升高度
            } else {
                block.bounceOffset = -4.0f * (block.bounceTimer / 5.0f); // 线性回落
            }
            
            if (block.bounceTimer == 0) {
                block.bounceOffset = 0.0f; // 强制归零，绝不漂移
            }
        }
    }
}

//将不同形态的地图元素（水管、砖块、地面等）提取出它们的物理边界（AABB），打上类型标签（SolidType），并记录原始索引，统一存入 m_SolidColliders 数组
void LevelManager::BuildSolidColliders() {
    m_SolidColliders.clear();
    //利用 Lambda 提取 bounds 和 索引
    auto inject = [this](const auto& elements, SolidType type) {
        for (int i = 0; i < elements.size(); ++i) {
            m_SolidColliders.push_back({
                elements[i].bounds,
                type, 
                (int)m_SolidColliders.size()                  // 记录原始索引，供后续交互使用
            });
        }
    };
    //批量提取
    inject(m_Pipes,   SolidType::PIPE);
    inject(m_Bricks,  SolidType::BRICK);
    inject(m_Boxes,   SolidType::BOX);
    // 处理 Ground 和 Step (它们本身就是 AABB，没有 .bounds，需特殊处理)
    for (int i = 0; i < m_Grounds.size(); ++i) {
        m_SolidColliders.push_back({m_Grounds[i], SolidType::GROUND, i});
    }
    for (int i = 0; i < m_Steps.size(); ++i) {
        m_SolidColliders.push_back({m_Steps[i], SolidType::STEP, i});
    }
}

//关卡加载
bool LevelManager::LoadLevel(const std::string& filePath) {
    std::ifstream file(filePath);
    //无法打开
    if (!file.is_open()) {
        std::cerr << "无法加载地图文件: " << filePath << std::endl;
        return false;
    }

    //异常处理
    try {
        
        json data = json::parse(file);//将 JSON 字符串解析为 JavaScript 对象或值
        m_Maps.clear(); m_Grounds.clear(); m_Steps.clear();
        m_Pipes.clear(); m_Coins.clear(); m_Bricks.clear();
        m_Boxes.clear(); m_Checkpoints.clear(); m_Flagpoles.clear();
        m_EnemyGroups.clear();
        m_ImageName = data["image_name"].get<std::string>();

        // Maps
        for (auto& m : data["maps"]) {
            m_Maps.push_back({m["start_x"], m["end_x"], m["player_x"], m["player_y"]});
        }

        // Ground
        for (auto& g : data["ground"]) {
            m_Grounds.push_back({g["x"], g["y"], g["width"], g["height"]});
        }

        // Step
        for (auto& s : data["step"]) {
            m_Steps.push_back({s["x"], s["y"], s["width"], s["height"]});
        }

        // Pipe
        for (auto& p : data["pipe"]) {
            m_Pipes.push_back({{p["x"], p["y"], p["width"], p["height"]}, p["type"]});
        }

        // Coin
        for (auto& c : data["coin"]) {
            m_Coins.push_back({c["x"], c["y"]});
        }

        // Brick (处理可选字段)
        for (auto& b : data["brick"]) {
            Brick brick;
            brick.bounds.x = b["x"]; 
            brick.bounds.y = b["y"];
            brick.bounds.width = b.value("width", 40.0f);  // 防止隐形碰撞体
            brick.bounds.height = b.value("height", 43.0f);
            brick.type = b["type"];
            brick.color = b.value("color", 0);
            brick.brick_num = b.value("brick_num", 0);
            brick.direction = b.value("direction", 0);
            m_Bricks.push_back(brick);
        }

        // Box
        for (auto& bx : data["box"]) {
            Box box;
            box.bounds.x = bx["x"]; 
            box.bounds.y = bx["y"];
            box.bounds.width = bx.value("width", 40.0f);  // 防止隐形碰撞体
            box.bounds.height = bx.value("height", 43.0f);
            box.type = bx["type"];
            m_Boxes.push_back(box);
        }

        // Enemy (特殊嵌套结构: [{"0": [...]}, {"1": [...]}])
        m_EnemyGroups.clear();
        for (auto& group_obj : data["enemy"]) {
            for (auto& [key, enemies_arr] : group_obj.items()) {
                int group_id = std::stoi(key); // "0" -> 0
                std::vector<EnemySpawn> group;
                for (auto& e : enemies_arr) {
                    group.push_back({e["x"], e["y"], e["direction"], e["type"], e["color"]});
                }
                m_EnemyGroups[group_id] = group;
            }
        }

        // Checkpoint
        for (auto& cp : data["checkpoint"]) {
            Checkpoint c;
            c.bounds.x = cp["x"]; 
            c.bounds.y = cp["y"]; 
            c.bounds.width = cp["width"]; 
            c.bounds.height = cp["height"];
            c.type = cp["type"];
            c.enemy_groupid = cp.value("enemy_groupid", -1);
            c.map_index = cp.value("map_index", -1);
            m_Checkpoints.push_back(c);
        }

        // Flagpole
        for (auto& fp : data["flagpole"]) {
            m_Flagpoles.push_back({fp["x"], fp["y"], fp["type"]});
        }

        BuildSolidColliders();
        return true;

    }
    catch (json::exception& e) {
        std::cerr << "JSON 解析错误: " << e.what() << std::endl;
        return false;
    }
}