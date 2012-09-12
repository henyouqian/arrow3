#ifndef __TASK_CUBE_H__
#define __TASK_CUBE_H__

#include "spritePod.h"

class Target;

class TaskCube : public lw::Task, public lw::Singleton<TaskCube>, public lw::ButtonCallback, public lw::CheckboxCallback{
public:
    virtual void vBegin();
    virtual void vEnd();
    virtual void vMain();
    virtual void vDraw();
    virtual bool vEvent(const lw::TouchEvent& evt);
    
private:
    virtual void vClick(lw::Button* pButton);
    virtual void vCheck(lw::Checkbox* pCb);
    
    lw::Sprite* _pSpt;
    float _t;
    lw::SoundSource* _pSnd;
    lw::Button* _pBtn;
    lw::Checkbox* _pCheckbox;
    SpritePod* _pSptPod;
    Target* _pTarget;
    b2World* _pWorld;
    lw::LabelBM *_pLabel;
    lw::AudioPlayer *_player;
};

#endif //__TASK_CUBE_H__