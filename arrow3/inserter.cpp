#include "stdafx.h"
#include "inserter.h"

Inserter::Inserter()
:_hit(false){
    
}

Inserter::~Inserter(){
    
}

void Inserter::main(){
    vMain();
}

void Inserter::draw(){
    vDraw();
}


//=======================================
RayInserter::RayInserter(const char *file, float u, float v, float w, float h, float ancX, float ancY)
:_speed(10.f){
    _pSpt = lw::Sprite::create(file);
    _pSpt->setUV(u, v, w, h);
    _pSpt->setAnchor(ancX, ancY);
}

RayInserter::~RayInserter(){
    delete _pSpt;
}

void RayInserter::setTransform(float posX, float posY, float rotate){
    _pSpt->setPos(posX, posY);
    _pSpt->setRotate(rotate);
}

void RayInserter::setSpeed(float speed){
    _speed = speed;
}

void RayInserter::vMain(){
    b2Vec2 v(0.f, 1.f);
    b2Rot rot(_pSpt->getRotate());
    v = b2Mul(rot, v);
    float x, y;
    _pSpt->getPos(x, y);
    _pSpt->setPos(x+v.x, y+v.y);
}

void RayInserter::vDraw(){
    _pSpt->draw();
}
