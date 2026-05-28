#include "Entity\GameObject.h"
#include "Utils\AABB.h"

GameObject::GameObject(float xx,float yy,int w,int h)
:
    x(xx)
    ,y(yy)
    ,width(w)
    ,height(h)
    ,vX(0.0f)
    ,vY(0.0f)
    ,isActive(true){}

void GameObject::Update(){
    if(!isActive)return;
    x += vX;
    y += vY;

}

RECT GameObject::GetBoundingBox()const{
    RECT box;

    box.left   = static_cast<LONG>(x);
    box.top    = static_cast<LONG>(y);
    box.right  = static_cast<LONG>(x + width);
    box.bottom = static_cast<LONG>(y + height);
    return box;
}