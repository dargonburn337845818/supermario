#pragma once
#include <string>
#include "component\PhysicsComponent.h"
#include "component\Animation.h"
#include "core\levelManager.h"
#include "Utils\AABB.h" 
class LevelManager;

class Mario {
public:
    Mario(float startX, float startY);
    ~Mario();

    void LoadResources(const std::string& imagePath, const std::string& jsonPath);
    void Update(const LevelManager& levelMgr);
    void Render(IMAGE* target); 

    float GetX()const { return x; }
    float GetY()const { return y; }
    float GetWidth() const { return m_Width; }
    bool isOnGround = true;
    bool isDead = false;
    bool isBig = false;
    bool isFire = false;
    bool facingRight = true;

private:
    PhysicsComponent physics; 
    Animation animation; 

    float x, y;
    float m_Width  = 16.0f;   // ★ 小马里奥逻辑宽度
    float m_Height = 16.0f;   // ★ 小马里奥逻辑高度

    int walkFrameTimer = 0; //动画计时器，固定帧数后切换动画帧
    int walkFrameIndex = 1; //动画帧索引

    bool isRunning = false;   //奔跑
    bool isBraking = false;   // 急刹
    bool isAttack = false;   // 坐到敌人

    struct SpeedConfig {
        float max_walk_speed = 1.5f;
        float max_run_speed = 3.0f;
        float walk_accel = 0.1f;
        float run_accel = 0.3f;
        float jump_velocity = -4.5f;
        float brake_accel = 0.2f;
    } speedConfig;
};