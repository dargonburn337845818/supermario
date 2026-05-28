#pragma once
#include <graphics.h>

class GameObject{
public:
    float x,y; //坐标
    int width,height;

    //矢量速度
    float vX,vY;

    //是否存活，非活放入死亡栈
    bool isActive;

    GameObject(float xx,float yy,int w,int h);
    virtual ~GameObject() = default;

    virtual void Update(); //更新（砖块有不更新的情况）
    virtual void Render() = 0; //强制渲染

    virtual RECT GetBoundingBox() const; //碰撞框
};