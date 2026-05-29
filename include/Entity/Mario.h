#pragma once
#include <string>
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
    void Render(); 

    bool isOnGround = true;
    bool isDead = false;
    bool isBig = false;
    bool isFire = false;
    bool facingRight = true;

private:
    Animation animation; 

    float x, y;
    float velocityX, velocityY;
    int width, height; // 碰撞箱
    
    int walkFrameTimer = 0; //动画计时器，固定帧数后切换动画帧
    int walkFrameIndex = 1; //动画帧索引

    // 新增状态标记
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
        float gravity = 0.25f;
        float max_y_velocity = 4.0f;
    } speedConfig;
};