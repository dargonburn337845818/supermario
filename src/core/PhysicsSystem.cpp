#include "component\PhysicsComponent.h"
#include "Utils\AABB.h"

void PhysicsSystem::Update(PhysicsComponent& physics, AABB& entityBox, const LevelManager& levelMgr) {
    // --- 1. 施加重力并更新 Y 轴位置 ---
    physics.velY += physics.gravity;
    if (physics.velY > 12.0f) physics.velY = 12.0f; // 限制最大下落速度，防止穿透薄地面
    entityBox.y += physics.velY;
    
    physics.isOnGround = false;
    physics.hitHead = false; // 每帧重置撞头状态，由碰撞检测重新赋值
    // LevelManager::BuildSolidColliders() 烘焙好的统一碰撞器列表
    const auto& colliders = levelMgr.GetSolidColliders();
    // --- 2. Y 轴碰撞检测 ---
    for (const auto& wall : colliders) {
        if (IsOverlapping(entityBox, wall.bounds)) {
            if (physics.velY > 0) { // 正在下落，脚着地
                physics.isOnGround = true;
                entityBox.y = wall.bounds.y - entityBox.height; // 贴合地面
                physics.velY = 0;
            } else if (physics.velY < 0) { // 正在上升，头顶撞墙
                entityBox.y = wall.bounds.y + wall.bounds.height; // 贴合天花板
                physics.velY = 0;
                
                // 记录撞头信息，将物理事件转化为游戏逻辑事件
                physics.hitHead = true;
                physics.hitHeadType = wall.type;   // 撞到了什么类型？(砖块/问号块/水管)
                physics.hitHeadIndex = wall.index; // 撞到了哪一个？(用于修改特定砖块的状态)
            }
        }
    }
    // --- 3. 更新 X 轴位置 ---
    entityBox.x += physics.velX;
    // --- 4. X 轴碰撞检测 ---
    for (const auto& wall : colliders) {
        if (IsOverlapping(entityBox, wall.bounds)) {
            if (physics.velX > 0) { // 向右移动撞墙
                entityBox.x = wall.bounds.x - entityBox.width;
            } else if (physics.velX < 0) { // 向左移动撞墙
                entityBox.x = wall.bounds.x + wall.bounds.width;
            }
            physics.velX = 0;
        }
    }
}