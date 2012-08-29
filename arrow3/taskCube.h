#ifndef __TASK_CUBE_H__
#define __TASK_CUBE_H__

#include "lwtf/UI/lwButton.h"
#include "spritePod.h"

class Target;

class TaskCube : public lw::Task, public lw::Singleton<TaskCube>, public lw::ButtonCallback{
public:
    virtual void vBegin();
    virtual void vEnd();
    virtual void vMain();
    virtual void vDraw();
    virtual void vEvent(const lw::TouchEvent& evt);
    
private:
    virtual void vClick(lw::Button* pButton);
    
    lw::Sprite* _pSpt;
    float _t;
    lw::SoundSource* _pSnd;
    lw::Button* _pBtn;
    SpritePod* _pSptPod;
    Target* _pTarget;
    b2World* _pWorld;
};

#endif //__TASK_CUBE_H__