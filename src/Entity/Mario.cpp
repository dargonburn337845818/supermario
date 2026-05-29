#include "Entity\Mario.h"
#include "Utils\json.hpp"
#include <graphics.h>
#include <fstream>
#include <iostream>
#include <cmath>

using json = nlohmann::json;

Mario::Mario(float startX, float startY)  {
    x = startX; y = startY;
    width = 16; height = 16; // 碰撞箱
    velocityX = 0; velocityY = 0;
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
            speedConfig.max_y_velocity = s.value("max_y_velocity", speedConfig.max_y_velocity);
            speedConfig.walk_accel     = s.value("walk_accel", speedConfig.walk_accel);
            speedConfig.run_accel      = s.value("run_accel", speedConfig.run_accel);
            speedConfig.jump_velocity  = s.value("jump_velocity", speedConfig.jump_velocity);
            speedConfig.brake_accel    = s.value("brake_accel", speedConfig.brake_accel);
            speedConfig.gravity        = s.value("gravity", speedConfig.gravity);
        }
    } catch (json::exception& e) {
        // 防御性编程：JSON格式错误不崩溃
        // OutputDebugStringA(e.what());
    }
}

void Mario::Update(const LevelManager& levelMgr) {
    if (isDead) {
        velocityY += speedConfig.gravity; // 死亡只受重力
        y += velocityY;
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
        if (velocityX < -0.1f) {
            // 当前正在向左移动，按右键 -> 视为急刹/减速
            velocityX += speedConfig.brake_accel; 
            if (isRunning && isOnGround) isBraking = true;
            // 注意：这里不改变 facingRight！继续保持面朝左滑行
        } else {
            // 速度已经降为0或正在向右移动 -> 正常加速，允许转向
            velocityX += currentAccel;
            facingRight = true; 
        }
    }
    if (inputLeft) {
        if (velocityX > 0.1f) {
            // 当前正在向右移动，按左键 -> 视为急刹/减速
            velocityX -= speedConfig.brake_accel;
            if (isRunning && isOnGround) isBraking = true;
            // 注意：这里不改变 facingRight！继续保持面朝右滑行
        } else {
            // 速度已经降为0或正在向左移动 -> 正常加速，允许转向
            velocityX -= currentAccel;
            facingRight = false; 
        }
    }
    
    if (inputJump && isOnGround) {
        velocityY = speedConfig.jump_velocity;
        isOnGround = false;
    }

    // 限制最大水平速度
    if (velocityX > currentMaxSpeed) velocityX = currentMaxSpeed;
    if (velocityX < -currentMaxSpeed) velocityX = -currentMaxSpeed;

    // 摩擦力 (松开按键时)
    bool isMoving = (inputLeft || inputRight);
    if (!isMoving && isOnGround) {
        velocityX *= 0.85f; 
        if (std::abs(velocityX) < 0.1f) velocityX = 0;
    }

    // 重力
    velocityY += speedConfig.gravity;
    if (velocityY > speedConfig.max_y_velocity) velocityY = speedConfig.max_y_velocity;

    // --- ★ 核心修改：分轴碰撞解决法 ★ ---
    isOnGround = false; // 每帧重置着地状态，由碰撞检测重新赋予
    // 1. Y 轴移动与碰撞
    y += velocityY;
    AABB marioBox = {x, y, (float)width, (float)height};
    for (const auto& wall : levelMgr.GetColliders()) {
        if (IsOverlapping(marioBox, wall.box)) {
            if (velocityY > 0) { // 掉落踩地
                y = wall.box.y - height; // 把脚底贴到墙顶
                velocityY = 0;
                isOnGround = true;
            } else if (velocityY < 0) { // 跳跃撞头
                y = wall.box.y + wall.box.height; // 把头顶贴到墙底
                velocityY = 0;
                // TODO: 如果 wall.type 是砖块/问号块，触发顶块逻辑
            }
            marioBox.y = y; // 更新碰撞箱，防止后续检测误判
        }
    }
    // 2. X 轴移动与碰撞
    x += velocityX;
    marioBox.x = x;
    for (const auto& wall : levelMgr.GetColliders()) {
        if (IsOverlapping(marioBox, wall.box)) {
            if (velocityX > 0) { // 向右撞墙
                x = wall.box.x - width;
                velocityX = 0;
            } else if (velocityX < 0) { // 向左撞墙
                x = wall.box.x + wall.box.width;
                velocityX = 0;
            }
            marioBox.x = x;
        }
    }

    //左端空气墙
    if (x < 0) {
        x = 0;
        velocityX = 0;
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
    }

    // 最终推送给动画组件
    animation.SetAnimSet(targetSet);
    animation.SetFrameIndex(targetFrame);
}

void Mario::Render() {
    // 获取当前帧宽高用于居中偏移
    int currentFrameWidth = animation.GetCurrentFrameWidth();
    int currentFrameHeight = animation.GetCurrentFrameHeight();

    // 居中绘制，防止不同帧宽高造成的抖动
    int drawX = (int)x + (width - currentFrameWidth) / 2; 
    int drawY = (int)y + (height - currentFrameHeight);   // 底部对齐

    // 传入 !facingRight 决定是否翻转 (向左时翻转)
    animation.Draw(drawX, drawY, !facingRight);
}