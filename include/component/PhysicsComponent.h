#pragma once
#include "core\LevelManager.h" //引入SolidType
#include "Utils\AABB.h"

struct PhysicsComponent {
    AABB bounds;
    float velX = 0, velY = 0; //矢量速度
    float gravity = 0.45f; //重力
    float max_y_velocity = 4.0f;
    bool isOnGround = false; //地面判定
    bool hitHead = false;   //撞头，与方块交互
    SolidType hitHeadType = SolidType::GROUND; //撞到的类型
    int hitHeadIndex = -1;        // 撞到的物体索引
};

class PhysicsSystem {
public:
    static void Update(PhysicsComponent& physics, AABB& entityBox, const LevelManager& levelMgr);
    //角色的物理状态（可能需要修改里面的速度、是否落地等状态）
    //角色的碰撞箱位置和大小（算出移动后的新位置，直接修改它）
    //问 LevelManager 要地图数据
};