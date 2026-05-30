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
    for (int i = 0; i < colliders.size(); ++i) {
        const auto& wall = colliders[i];
        if (IsOverlapping(entityBox, wall.bounds)) {
            if (physics.velY > 0) { 
                physics.isOnGround = true;
                entityBox.y = wall.bounds.y - entityBox.height; 
                physics.velY = 0;
            } else if (physics.velY < 0) { 
                entityBox.y = wall.bounds.y + wall.bounds.height; 
                physics.velY = 0;
                
                physics.hitHead = true;
                physics.hitHeadType = wall.type;   
                physics.hitHeadIndex = i;  // 直接使用大数组的真实下标 i
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