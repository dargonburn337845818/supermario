#pragma once

class Camera {
public:
    Camera() = default;

    // 传入物理窗口尺寸 + 关卡逻辑尺寸
    void Init(int screenW, int screenH, int mapW, int mapH);

    void Update(float targetX, float targetWidth = 0.0f);
    float GetX() const;
    float GetRenderOffsetX() const;

    // 渲染时 StretchBlt 需要
    float GetScale() const { return m_Scale; }
    int   GetViewW() const { return m_ViewW; }   // 逻辑视口宽
    int   GetViewH() const { return m_ViewH; }   // 逻辑视口高

private:
    float x = 0.0f;         // 摄像机世界X（逻辑坐标）

    int   m_ScreenW = 0;    // 物理窗口宽  (800)
    int   m_ScreenH = 0;    // 物理窗口高  (600)
    int   m_MapW    = 0;    // 关卡逻辑宽  (3390)
    int   m_MapH    = 0;    // 关卡逻辑高  (224)

    float m_Scale = 1.0f;   // 逻辑→屏幕 缩放比
    int   m_ViewW = 0;      // 逻辑视口宽  (≈297)
    int   m_ViewH = 0;      // 逻辑视口高  (=223)
};