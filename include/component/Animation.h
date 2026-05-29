#pragma once
#include <graphics.h>
#include <string>
#include <map>
#include <vector>
#include "Utils\json.hpp"

// 单帧的数据
struct Frame {
    int x;
    int y;
    int width;
    int height;
    IMAGE* imgOriginal = nullptr; // 预切好的向右原图
    IMAGE* imgFlipped = nullptr;  // 预切好的向左翻转图
};

class Animation {
public:
    Animation();
    ~Animation();

    //专门用于绘制带 Alpha 通道的透明图片
    void DrawAlpha(int x, int y, IMAGE* img);

    void Load(const std::string& imagePath, const std::string& jsonPath); //读取所有动画集数据

    void SetAnimSet(const std::string& setName); //根据状态选择动画集

    void SetFrameIndex(int index); //设置帧

    void Draw(int x, int y, bool flip = false); //决定左右动画集

    int GetCurrentFrameWidth() const;
    int GetCurrentFrameHeight() const;

private:
    IMAGE spriteSheet;                                  
    std::map<std::string, std::vector<Frame>> animSets; //存储不同状态的动画集（键名->框架）
    
    std::string currentSetName; //当前动画集键名                    
    std::vector<Frame>* currentSet; //当前动画集        
    int currentFrameIndex; //当前动画集在第几帧        

    // 辅助函数：生成水平翻转的 IMAGE
    void FlipImage(const IMAGE* src, IMAGE* dest); 

    // 辅助函数：清理所有 new 出来的 IMAGE
    void ClearFrames(); 
};