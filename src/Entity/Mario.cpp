#include "Entity\Mario.h"
#include "component\PhysicsComponent.h" 
#include "Utils\json.hpp"
#include <graphics.h>
#include <fstream>
#include <iostream>
#include <cmath>

using json = nlohmann::json;

Mario::Mario(float startX, float startY)  {
    x = startX; y = startY;
    physics.bounds = {x, y, 32.0f, 32.0f}; 
}

Mario::~Mario() {}

void Mario::LoadResources(const std::string& imagePath, const std::string& jsonPath) {
    animation.Load(imagePath, jsonPath);
    
    std::ifstream file(jsonPath);
    if (!file.is_open()) return;

    try {
        json data = json::parse(file);
        if (data.contains("speed")) {
            auto& s = data["speed"];
            // 使用 value 方法读取，如果 JSON 缺失则使用 struct 里的默认值兜底
            speedConfig.max_walk_speed = s.value("max_walk_speed", speedConfig.max_walk_speed);
            speedConfig.max_run_speed  = s.value("max_run_speed", speedConfig.max_run_speed);
            physics.max_y_velocity = s.value("max_y_velocity", physics.max_y_velocity);
            speedConfig.walk_accel     = s.value("walk_accel", speedConfig.walk_accel);
            speedConfig.run_accel      = s.value("run_accel", speedConfig.run_accel);
            speedConfig.jump_velocity  = s.value("jump_velocity", speedConfig.jump_velocity);
            speedConfig.brake_accel    = s.value("brake_accel", speedConfig.brake_accel);
            physics.gravity        = s.value("gravity", physics.gravity);
        }
    } catch (json::exception& e) {
        // 防御性编程：JSON格式错误不崩溃
        // OutputDebugStringA(e.what());
    }
}

void Mario::Update(LevelManager& levelMgr) {
    if (isDead) {
        physics.velY += physics.gravity;  // 死亡只受重力
        physics.bounds.y += physics.velY;
        y = physics.bounds.y;  
        animation.SetAnimSet("right_small_die");
        animation.SetFrameIndex(0);
        return;
    }

    // 1.输入与物理逻辑
    bool inputLeft = (GetAsyncKeyState(VK_LEFT) & 0x8000);
    bool inputRight = (GetAsyncKeyState(VK_RIGHT) & 0x8000);
    bool inputJump = (GetAsyncKeyState(VK_SPACE) & 0x8000);

    bool isRunning = (GetAsyncKeyState(VK_SHIFT) & 0x8000); 

    float currentAccel = isRunning ? speedConfig.run_accel : speedConfig.walk_accel;
    float currentMaxSpeed = isRunning ? speedConfig.max_run_speed : speedConfig.max_walk_speed;
    isBraking = false;

    if (inputRight) {
        if (physics.velX < -0.1f) {
            // 当前正在向左移动，按右键 -> 视为急刹/减速
            physics.velX += speedConfig.brake_accel; 
            if (isRunning && isOnGround) isBraking = true;
            // 注意：这里不改变 facingRight！继续保持面朝左滑行
        } else {
            // 速度已经降为0或正在向右移动 -> 正常加速，允许转向
            physics.velX += currentAccel;
            facingRight = true; 
        }
    }
    if (inputLeft) {
        if (physics.velX > 0.1f) {
            // 当前正在向右移动，按左键 -> 视为急刹/减速
            physics.velX -= speedConfig.brake_accel;
            if (isRunning && isOnGround) isBraking = true;
            // 注意：这里不改变 facingRight！继续保持面朝右滑行
        } else {
            // 速度已经降为0或正在向左移动 -> 正常加速，允许转向
            physics.velX -= currentAccel;
            facingRight = false; 
        }
    }
    
    if (inputJump && isOnGround) {
        physics.velY = speedConfig.jump_velocity;
        isOnGround = false;
    }

    // 限制最大水平速度
    if (physics.velX > currentMaxSpeed) physics.velX = currentMaxSpeed;
    if (physics.velX < -currentMaxSpeed) physics.velX = -currentMaxSpeed;

    // 摩擦力 (松开按键时)
    bool isMoving = (inputLeft || inputRight);
    if (!isMoving && isOnGround) {
        physics.velX *= 0.85f; 
        if (std::abs(physics.velX) < 0.1f) physics.velX = 0;
    }

    // --- ★ 核心重构：将位置和碰撞全权交给 PhysicsSystem ★ ---
    // 保持物理组件的坐标与 Mario 的逻辑坐标同步
    physics.bounds.x = x;
    physics.bounds.y = y;
    // 调用物理系统更新（处理重力、移动、碰撞回推）
    PhysicsSystem::Update(physics, physics.bounds, levelMgr);
    // 从物理系统取回计算后的坐标
    x = physics.bounds.x;
    y = physics.bounds.y;
    isOnGround = physics.isOnGround;
    // 处理物理系统抛出的撞头事件
    if (physics.hitHead) {
        // 将撞击事件交给 LevelManager 处理
        levelMgr.OnBlockHit(physics.hitHeadIndex, physics.hitHeadType);
    }
    // 左端空气墙
    if (x < 0) {
        x = 0; 
        physics.velX = 0;
        physics.bounds.x = 0;
    }

    // 2.动画状态机
    std::string targetSet = "right_small_stay"; //待机状态
    int targetFrame = 0; 

    if (!isOnGround) { // 跳跃 (向上)
        // 跳跃 (上升期)
        targetSet = "right_small_jump";
        // JSON中 jump 只有1帧 (索引0)
        targetFrame = 0; 
        walkFrameIndex = 0; // 重置，为落地走路做准备
        walkFrameTimer = 0;
    }
    else if (isBraking) {
        targetSet = "right_small_stop";
        // JSON中 stop 只有1帧 (索引0)
        targetFrame = 0; 
    }
    else if (isMoving) {
        targetSet = "right_small_walk";
        // JSON中 walk 有3帧 (索引0, 1, 2)
        int animSpeed = isRunning ? 4 : 6; // 奔跑时动画加快
        walkFrameTimer++;
        if (walkFrameTimer > animSpeed) { 
            walkFrameTimer = 0; 
            walkFrameIndex++; 
            if (walkFrameIndex > 2) walkFrameIndex = 0; // 超过2回到0
        }
        targetFrame = walkFrameIndex;
    }
    else {
        targetSet = "right_small_stay";
        // JSON中 stay 只有1帧 (索引0)
        targetFrame = 0; 
        walkFrameIndex = 0; // 重置，为下次起步做准备
        walkFrameTimer = 0;
    }

    // 最终推送给动画组件
    animation.SetAnimSet(targetSet);
    animation.SetFrameIndex(targetFrame);
}

void Mario::Render(IMAGE* target) {
    animation.Draw(target, (int)x, (int)y, (int)m_Width, (int)m_Height, !facingRight); 
}