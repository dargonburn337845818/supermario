#pragma once
#include <graphics.h>
namespace AABB {
    // 检测两个矩形是否发生重叠
    inline bool CheckCollision(const RECT& a, const RECT& b) {
        return (a.left < b.right && 
                a.right > b.left && 
                a.top < b.bottom && 
                a.bottom > b.top);
    }
    
    // 后续还可以扩展：计算重叠面积、获取碰撞方向等
}