#include "Entity\Mario.h"
#include "Utils\json.hpp" // 引入 JSON 库
#include <graphics.h>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

Mario::Mario(float x, float y) : GameObject(x, y, 16, 16) { // 马里奥原版是16x16，先改小碰撞箱
    currentFrame = 0;
    frameCounter = 0;
    isMoving = false;
}

void Mario::LoadResources(const std::string& imagePath, const std::string& jsonPath) {
    // 1. 加载精灵图
    loadimage(&spriteSheet, imagePath.c_str());
    
    // 2. 解析 JSON
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        std::cerr << "Failed to load JSON: " << jsonPath << std::endl;
        return;
    }
    json data = json::parse(file);

    // 3. 读取 right_small_normal 帧
    for (const auto& f : data["image_frames"]["right_small_normal"]) {
        frames_right_small_normal.push_back({f["x"], f["y"], f["width"], f["height"]});
    }

    // 4. 读取物理速度数据
    speedConfig.max_walk_speed = data["speed"]["max_walk_speed"];
    speedConfig.walk_accel = data["speed"]["walk_accel"];
    speedConfig.jump_velocity = data["speed"]["jump_velocity"];
    // ... 其他数据按需读取
}

void Mario::Update() {
    isMoving = false;

    // --- 使用 JSON 里的物理配置替代硬编码 ---
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        vX += speedConfig.walk_accel; // 使用加速度
        if (vX > speedConfig.max_walk_speed) vX = speedConfig.max_walk_speed; // 限制最大速度
        isMoving = true;
    } 
    else if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        vX -= speedConfig.walk_accel;
        if (vX < -speedConfig.max_walk_speed) vX = -speedConfig.max_walk_speed;
        isMoving = true;
    } 
    else {
        // 简单的摩擦力逻辑：松开按键后逐渐减速
        if (vX > 0) vX -= 0.2f;
        if (vX < 0) vX += 0.2f;
        if (abs(vX) < 0.2f) vX = 0.0f;
    }

    // 简单的帧动画切换逻辑
    if (isMoving) {
        frameCounter++;
        if (frameCounter > 6) { // 每 6 帧切换一次动画图片
            currentFrame = (currentFrame + 1) % frames_right_small_normal.size();
            frameCounter = 0;
        }
    } else {
        currentFrame = 0; // 静止时显示第一帧（站立）
        frameCounter = 0;
    }

    GameObject::Update(); // x += vX
}

void Mario::Render() {
    if (frames_right_small_normal.empty()) return; // 安全检查
    
    // 获取当前帧的数据
    Frame& f = frames_right_small_normal[currentFrame];
    
    // 由于马里奥不同帧的宽度不一样（有的是12，有的是16），
    // 为了防止走路时图片左右抖动，我们需要把图片居中绘制在碰撞箱内
    int drawX = x + (width - f.width) / 2; 
    
    // EasyX 核心绘图函数：从大图中裁剪出小图绘制
    // putimage(目标x, 目标y, 裁剪宽, 裁剪高, 源图片指针, 源图裁剪起始x, 源图裁剪起始y)
    putimage(drawX, y, f.width, f.height, &spriteSheet, f.x, f.y);
}