#include "stdafx.h"
#include "taskCube.h"


void TaskCube::vBegin(){
    _pSpt = lw::Sprite::create("girl0.png");
    _pSpt->setAnchor(160.f, 240.f);
    _pSpt->setPos(160, 240.f);
    _pSpt->setBlendMode(lw::BLEND_ADD);
    //_pSpt->setUV(100, 100, 20, 20);
    //_pSpt->setSize(100.f, 100.f);
    //_pSpt->setColor(lw::Color(1.f, 1.f, 0.f, 0.5f));
    _t = 0;
    
    _pSnd = lw::SoundSource::create("success.wav", 2, false);
    
    _pBtn = lw::Button::create(this, NULL, "girl0.png", 100, 100, 100, 100, 130, 100, 100, 100);
    
    _pSptPod = new SpritePod("fluit.pod", "apple");
    _pSptPod->setPos(100, 200);
}

void TaskCube::vEnd(){
    delete _pSpt;
    delete _pSnd;
    delete _pBtn;
    delete _pSptPod;
}

void TaskCube::vMain(){
    
}

void TaskCube::vDraw(){
    glClearColor(0.65f, 0.65f, 0.65f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    _t+=.1f;
    float s = sinf(_t);
    _pSpt->setRotate(_t*.1f);
    _pSpt->setScale(s, s);
    //_pSpt->draw();
    _pBtn->draw();
    _pSptPod->draw();
}

void TaskCube::vEvent(const lw::TouchEvent& evt){
    if ( evt.type == lw::TouchEvent::TOUCH ){
        lwinfo("touch");
        //_pSnd->play();
    }else if ( evt.type == lw::TouchEvent::MOVE ){
        lwinfo("move");
    }else if ( evt.type == lw::TouchEvent::UNTOUCH ){
        lwinfo("untouch");
    }
    _pBtn->event(evt);
}

void TaskCube::vClick(lw::Button* pButton){
    if ( pButton == _pBtn ){
        _pSnd->play();
    }
}