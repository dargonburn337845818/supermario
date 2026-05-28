#pragma once

class Camera {
public:
    // screenWidth: 游戏窗口宽度, mapWidth: 关卡地图总宽度
    Camera(int screenWidth, int mapWidth);

    // targetX: 跟随目标(马里奥)的X坐标, targetWidth: 目标的宽度
    void Update(float targetX, float targetWidth = 0.0f);

    // 获取摄像机视口左边缘在世界坐标系中的位置
    float GetX() const;

    // 获取渲染偏移量 (用于将世界坐标转换为屏幕坐标)
    float GetRenderOffsetX() const;

private:
    float x;           // 摄像机在世界坐标系中的X位置
    int screenWidth;   // 视口宽度
    int mapWidth;      // 地图总宽度
};