#include "core\PhysicsSystem.h"

void PhysicsSystem::Update(PhysicsComponent& physics, AABB& entityBox, const LevelManager& levelMgr) {
    // --- 1. 施加重力并更新 Y 轴位置 ---
    physics.velY += physics.gravity;
    entityBox.y += physics.velY;
    
    physics.isOnGround = false;

    // --- 2. Y 轴碰撞检测 ---
    for (const auto& wall : levelMgr.GetColliders()) {
        if (IsOverlapping(entityBox, wall.box)) {
            // 发生碰撞，判断方向
            if (physics.velY > 0) { 
                // 向下掉落时碰撞 => 踩地
                physics.isOnGround = true;
                entityBox.y = wall.box.y - entityBox.height; // 推回到地面正上方
                physics.velY = 0;
            } else if (physics.velY < 0) { 
                // 向上跳跃时碰撞 => 撞头/顶砖块
                entityBox.y = wall.box.y + wall.box.height; // 推回到方块正下方
                physics.velY = 0;
                // TODO: 触发顶砖块逻辑
            }
        }
    }

    // --- 3. 更新 X 轴位置 ---
    entityBox.x += physics.velX;

    // --- 4. X 轴碰撞检测 ---
    for (const auto& wall : levelMgr.GetColliders()) {
        if (IsOverlapping(entityBox, wall.box)) {
            if (physics.velX > 0) { 
                // 向右走撞墙
                entityBox.x = wall.box.x - entityBox.width;
            } else if (physics.velX < 0) { 
                // 向左走撞墙
                entityBox.x = wall.box.x + wall.box.width;
            }
            physics.velX = 0;
        }
    }
}