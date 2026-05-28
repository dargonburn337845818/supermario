#include "Entity\Mario.h"
#include <graphics.h>

Mario::Mario(float x, float y) : GameObject(x, y, 32, 32) {
    // 假设马里奥 32x32 像素大小
}

void Mario::Update() {
    // 暂时先在这里处理输入，清单第3步我们会把输入逻辑剥离得更优雅
    // 0x8000 检测按键是否正在被按住
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        vX = -3.0f; // 向左移动
    } 
    else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        vX = 3.0f;  // 向右移动
    } 
    else {
        vX = 0.0f;  // 松开按键就停止
    }

    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        vY = 3.0f; // 向下移动
    } 
    else if (GetAsyncKeyState(VK_UP) & 0x8000) {
        vY = -3.0f;  // 向上移动
    } 
    else {
        vY = 0.0f;  // 松开按键就停止
    }

    // 调用基类的 Update，让速度真正作用于坐标 (x += vX)
    GameObject::Update();
}

void Mario::Render() {
    // 设置填充色为红色
    setfillcolor(RED);
    // 在当前 (x, y) 位置画一个宽高为 width, height 的方块
    fillrectangle(x, y, x + width, y + height);
}