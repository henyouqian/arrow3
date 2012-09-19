#include "stdafx.h"
#include "taskCube.h"
#include "target.h"
#include "httpMsg.h"
#include "taskPoker.h"

void TaskCube::vBegin(){
    _pSpt = lw::Sprite::createFromFile("girl0.png");
    _pSpt->setAnchor(160.f, 240.f);
    _pSpt->setPos(160, 240.f);
    //_pSpt->setUV(0, 0, 320, 480);
    _pSpt->setBlendMode(lw::BLEND_ADD);
    //_pSpt->setUV(100, 100, 20, 20);
    //_pSpt->setSize(100.f, 100.f);
    //_pSpt->setColor(lw::Color(1.f, 1.f, 0.f, 0.5f));
    _t = 0;
    
    _pSnd = lw::SoundSource::create("success.wav", 2, false);
    
    _pBtn = lw::Button::create(this, NULL, "girl0.png", 100, 100, 130, 100, 100, 100, 100, 100);
    _pCheckbox = lw::Checkbox::create(this, NULL, "girl0.png", 100, 100, 130, 100, 100, 100, 100, 100);
    _pCheckbox->setPos(200, 0);
    
    _pSptPod = new SpritePod("fruit.pod", "apple");
    _pSptPod->setPos(100, 200);
    
    _pWorld = new b2World(b2Vec2(0, -10));
    
    b2BodyDef bd;
	bd.type = b2_dynamicBody;
	bd.allowSleep = false;
	bd.linearDamping = 0.3f;
	bd.angularDamping = 2.2f;
	bd.bullet = true;
    
    b2FixtureDef fd;
    fd.density = 1.f;
    fd.filter.groupIndex = -1;
    fd.friction = 0.2f;
    fd.restitution = 0.5f;
    
    _pTarget = new Target("fruit.pod", "apple", _pWorld, bd, fd);
    _pTarget->setPos(160.f, 0.f);
    
    _pLabel = lw::LabelBM::create("a.fnt");
    _pLabel->setAlign(lw::ALIGN_TOP_MID);
    _pLabel->setText("我靠\nasdakjl");
    _pLabel->setColor(lw::COLOR_BLACK);
    _pLabel->setPos(200, 200);
    _pLabel->setRotate(M_PI/3);
    _pLabel->setScale(.5f, 2.f);
    
    _player = lw::AudioPlayer::create("success.wav");

}

void TaskCube::vEnd(){
    delete _pSpt;
    delete _pSnd;
    delete _pBtn;
    delete _pCheckbox;
    delete _pSptPod;
    delete _pWorld;
    delete _pLabel;
    delete _player;
}

void TaskCube::vMain(){
    _pWorld->Step(1.f/60.f, 8, 3);
    
    static float f = 0;
    f += .01f;
    _pLabel->setRotate(f);
}

void TaskCube::vDraw(){
    glClearColor(0.65f, 0.65f, 0.65f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    _t+=.01f;
    float s = sinf(_t);
    _pSpt->setRotate(_t*.1f);
    _pSpt->setScale(s, s);
    _pSpt->draw();
    _pSptPod->draw();
    _pTarget->draw();
    _pLabel->draw();
}

bool TaskCube::vEvent(const lw::TouchEvent& evt){
    if ( evt.type == lw::TouchEvent::TOUCH ){
        lwinfo("touch");
        _player->setNumberOfLoops(1);
        _player->play();
        //_pSnd->play();
    }else if ( evt.type == lw::TouchEvent::MOVE ){
        lwinfo("move");
    }else if ( evt.type == lw::TouchEvent::UNTOUCH ){
        lwinfo("untouch");
    }

    return false;
}

void TaskCube::vClick(lw::Button* pButton){
    if ( pButton == _pBtn ){
        //_pSnd->play();
        lw::HTTPMsg* pMsg = new MsgLogin("12345", "李炜");
        pMsg->send();
    }
}

void TaskCube::vCheck(lw::Checkbox* pCb){
    switchTo(TaskPoker::ps());
    //stop();
    //TaskPoker::s().start();
}