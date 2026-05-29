#pragma once
#include "../Utils/AABB.h"

struct PhysicsComponent {
    float velX = 0;
    float velY = 0;
    float gravity = 0.5f;
    bool isOnGround = false;
    // AABB collisionBox; // 也可以挂载在组件里，或者 TransformComponent 里
};