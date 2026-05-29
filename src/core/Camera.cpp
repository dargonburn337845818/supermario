#include "core\Camera.h"
#include <algorithm>

void Camera::Init(int screenW, int screenH, int mapW, int mapH) {
    m_ScreenW = screenW;
    m_ScreenH = screenH;
    m_MapW    = mapW;
    m_MapH    = mapH;

    // 以高度为基准等比缩放，保证纵向完整显示
    m_Scale = (float)m_ScreenH / m_MapH;        // 600/223 ≈ 2.69

    // 反推逻辑视口能看多宽
    m_ViewW = (int)(m_ScreenW / m_Scale);        // 800/2.69 ≈ 297
    m_ViewH = m_MapH;                             // 纵向全显

    // 防止关卡比视口还窄
    if (m_ViewW > m_MapW) m_ViewW = m_MapW;

    x = 0.0f;
}

void Camera::Update(float targetX, float targetWidth) {

    // 1. 目标中心
    float targetCenterX = targetX + targetWidth / 2.0f;

    // 2. 视口中心
    float cameraCenterX = x + m_ViewW / 2.0f;

    // 3. 核心：只向右跟随，永不向左
    if (targetCenterX > cameraCenterX) {
        x = targetCenterX - m_ViewW / 2.0f;
    }

    // 4. 右边界
    float maxCamX = static_cast<float>(m_MapW - m_ViewW);
    if (x > maxCamX) x = maxCamX;

    // 5. 左边界
    if (x < 0.0f) x = 0.0f;
}

float Camera::GetX() const {
    return x;
}

float Camera::GetRenderOffsetX() const {
    return -x;
}