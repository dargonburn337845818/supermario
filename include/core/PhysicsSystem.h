#pragma once
#include "component\PhysicsComponent.h"
#include "Utils\AABB.h"
#include "core\levelManager.h"

class PhysicsSystem {
public:
    // 传入实体的碰撞箱和物理组件，以及地图碰撞数据
    static void Update(PhysicsComponent& physics, AABB& entityBox, const LevelManager& levelMgr);
};