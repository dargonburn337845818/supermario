#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "Utils\AABB.h"
#include <graphics.h>

// 地图区域/重生点
struct MapArea {
    int start_x, end_x, player_x, player_y;
};

// 水管 
struct Pipe {
    AABB bounds; 
    int type; // 0:普通 1:可进入 2:小管
};

// 金币 
struct Coin {
    float x, y;
};

// 砖块
struct Brick {
    AABB bounds;
    int type;
    int color = 0;
    int brick_num = 0;
    int direction = 0;
};

// 问号块
struct Box {
    AABB bounds;
    int type; // 1:金币 3:星星 4:蘑菇
};

// 敌人生成点
struct EnemySpawn {
    float x, y;
    int direction, type, color;
};

// 检查点/触发器
struct Checkpoint {
    AABB bounds;
    int type;
    int enemy_groupid = -1; //敌人触发线
    int map_index = -1;     //传送触发器
};

// 旗杆
struct Flagpole {
    float x, y;
    int type; // 0:底座 1:杆身 2:旗帜
};

enum class SolidType { GROUND, STEP, PIPE, BRICK, BOX };
struct SolidCollider {
    AABB bounds;
    SolidType type;
    int index; // 记录它在原始数组中的索引，方便后续交互
    bool isHit = false;
    float bounceOffset = 0.0f; // Y轴弹跳偏移
    int bounceTimer = 0;
};

struct CoinEffect {
    float x, y;
    float velY = -8.0f; // 向上弹射的初速度
    int life = 30;      // 30帧后消失
};

class LevelManager {
public:
    LevelManager();
    bool LoadLevel(const std::string& filePath);
    void OnBlockHit(int index, SolidType type); //头顶方块

    // 解析数据存储 (注意 Ground 和 Step 类型的变化)
    const std::string& GetImageName() const { return m_ImageName; }
    const std::vector<MapArea>& GetMaps() const { return m_Maps; }
    const std::vector<AABB>& GetGrounds() const { return m_Grounds; }   
    const std::vector<AABB>& GetSteps() const { return m_Steps; }        
    const std::vector<Pipe>& GetPipes() const { return m_Pipes; }
    const std::vector<Coin>& GetCoins() const { return m_Coins; }
    const std::vector<Brick>& GetBricks() const { return m_Bricks; }
    const std::vector<Box>& GetBoxes() const { return m_Boxes; }
    const std::unordered_map<int, std::vector<EnemySpawn>>& GetEnemyGroups() const { return m_EnemyGroups; }
    const std::vector<Checkpoint>& GetCheckpoints() const { return m_Checkpoints; }
    const std::vector<Flagpole>& GetFlagpoles() const { return m_Flagpoles; }
    const std::vector<CoinEffect>& GetCoinEffects() const { return m_CoinEffects; }

    // 让物理系统拿到所有参与碰撞的物体的统一列表
    const std::vector<SolidCollider>&GetSolidColliders() const {
        return m_SolidColliders;
    }

    // 撞击指定索引的方块
    void HitBlock(int index) {
        if (index >= 0 && index < m_SolidColliders.size()) {
            m_SolidColliders[index].isHit = true;
        }
    }
    
    // 检查方块是否已被撞击（防止重复顶出金币）
    bool IsBlockHit(int index) const {
        if (index >= 0 && index < m_SolidColliders.size()) {
            return m_SolidColliders[index].isHit;
        }
        return true; // 越界当已撞处理
    }

    void SpawnCoinEffect(int blockIndex);
    void UpdateEffects(); // 每帧调用

private:

    void BuildSolidColliders(); //碰撞数据烘焙管线
    std::string m_ImageName;
    std::vector<MapArea> m_Maps;
    std::vector<AABB> m_Grounds;                                    
    std::vector<AABB> m_Steps;                                
    std::vector<Pipe> m_Pipes;
    std::vector<Coin> m_Coins;
    std::vector<Brick> m_Bricks;
    std::vector<Box> m_Boxes;
    std::unordered_map<int, std::vector<EnemySpawn>> m_EnemyGroups; 
    std::vector<Checkpoint> m_Checkpoints;
    std::vector<Flagpole> m_Flagpoles;
    
    std::vector<SolidCollider> m_SolidColliders;
    std::vector<CoinEffect> m_CoinEffects;
    
};