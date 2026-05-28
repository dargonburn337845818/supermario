#pragma once
#include "GameObject.h"

class Mario : public GameObject {
public:
    Mario(float x, float y);
    
    // 实现基类的纯虚函数
    void Update() override;
    void Render() override;
};