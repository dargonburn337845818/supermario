#pragma once

struct AABB {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

// 碰撞检测辅助函数：判断两个AABB是否重叠
inline bool IsOverlapping(const AABB& a, const AABB& b) {
    return (a.x < b.x + b.width && a.x + a.width > b.x &&
            a.y < b.y + b.height && a.y + a.height > b.y);
}