#include "core\Camera.h"
#include <algorithm>

Camera::Camera(int screenWidth, int mapWidth) 
    : x(0.0f), screenWidth(screenWidth), mapWidth(mapWidth) {}

void Camera::Update(float targetX, float targetWidth) {
    // 1. 计算目标的中心点X坐标
    float targetCenterX = targetX + targetWidth / 2.0f;
    
    // 2. 计算当前摄像机视口的中心点X坐标
    float cameraCenterX = x + screenWidth / 2.0f;

    // 3. 核心逻辑：仅当目标超过视口中心点时，摄像机才向右跟随
    // 保证了摄像机永远不会向左移动
    if (targetCenterX > cameraCenterX) {
        x = targetCenterX - screenWidth / 2.0f;
    }

    // 4. 边界限制：摄像机视口不能超出地图右边缘
    float maxCamX = static_cast<float>(mapWidth - screenWidth);
    if (x > maxCamX) {
        x = maxCamX;
    }

    // 5. 边界限制：摄像机视口不能超出地图左边缘 (起点)
    if (x < 0.0f) {
        x = 0.0f;
    }
}

float Camera::GetX() const {
    return x;
}

float Camera::GetRenderOffsetX() const {
    // 世界坐标转屏幕坐标的偏移量
    // 屏幕X = 物体世界X - 摄像机世界X
    return -x; 
}