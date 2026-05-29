#pragma once
#include "Utils\AABB.h"
#include "Utils\json.hpp" 
#include <vector>
#include <string>

// 碰撞体类型，方便物理系统区分
enum class ColliderType { GROUND, PIPE, STEP, BRICK, BOX };

struct LevelCollider {
    AABB box;    
    ColliderType type;
    int extraData;    // 预留字段 (比如砖块的 type，水管的类型)
};

class LevelManager {
public:
    bool LoadLevel(const std::string& jsonPath);
    void Clear();

    const std::vector<LevelCollider>& GetColliders() const { return m_Colliders; }
    float GetLevelWidth() const { return m_LevelWidth; }

private:
    std::vector<LevelCollider> m_Colliders;
    float m_LevelWidth = 0;
    // 其他关卡数据 (敌人、硬币等) 后续再扩展
};