#pragma once
#include "GameObject.h"
#include <graphics.h>
#include <vector>
#include <string>

// 对应 JSON 里的每一帧
struct Frame {
    int x;
    int y;
    int width;
    int height;
};

// 对应 JSON 里的 speed 节点
struct SpeedConfig {
    float max_walk_speed;
    float max_run_speed;
    float max_y_velocity;
    float walk_accel;
    float run_accel;
    float jump_velocity;
};

class Mario : public GameObject {
public:
    Mario(float x, float y);
    
    // 新增：加载图片和 JSON 配置
    void LoadResources(const std::string& imagePath, const std::string& jsonPath);
    
    void Update() override;
    void Render() override;

private:
    IMAGE spriteSheet; // 精灵图表
    
    // 动画数据 (目前只存小马里奥向右的，后续可扩展向左、大马里奥等)
    std::vector<Frame> frames_right_small_normal;
    
    // 物理数据 (从 JSON 读取)
    SpeedConfig speedConfig;
    
    // 动画控制
    int currentFrame;   // 当前播放到哪一帧
    int frameCounter;   // 帧计数器，用于控制动画切换速度
    bool isMoving;      // 是否正在移动
};