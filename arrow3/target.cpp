#include "stdafx.h"
#include "target.h"
#include "spritePod.h"
#include "inserter.h"

const float B2_SCALE = 0.01f;

Target::Target(const char *file, const char *nodeName, b2World* pWorld, b2BodyDef &bodyDef, b2FixtureDef &fixtureDef)
:_pWorld(pWorld){
    _pSptPod = new SpritePod(file, nodeName);
    
    bodyDef.position.y = -bodyDef.position.y;
	_pBody = _pWorld->CreateBody(&bodyDef);
    
    const std::vector<lw::SpriteVertex>& vtxs = _pSptPod->getVertices();
	b2PolygonShape polygonShape;
	b2Vec2 b2v[3];
	for ( int i = 0; i < (int)vtxs.size(); ){
		b2v[0].Set(vtxs[i].x*B2_SCALE*0.9f, -vtxs[i].y*B2_SCALE*0.9f);
		++i;
		b2v[1].Set(vtxs[i].x*B2_SCALE*0.9f, -vtxs[i].y*B2_SCALE*0.9f);
		++i;
		b2v[2].Set(vtxs[i].x*B2_SCALE*0.9f, -vtxs[i].y*B2_SCALE*0.9f);
		++i;
		polygonShape.Set(b2v, 3);
        fixtureDef.shape = &polygonShape;
        fixtureDef.isSensor = true;
		_pBody->CreateFixture(&fixtureDef);
	}
}

Target::~Target(){
    delete _pSptPod;
    _pWorld->DestroyBody(_pBody);
}

void Target::draw(){
    if ( _pBody->GetType() == b2_dynamicBody ){
        const b2Transform& trans = _pBody->GetTransform();
        float x = trans.p.x/B2_SCALE;
        float y = trans.p.y/B2_SCALE;
        float r = trans.q.GetAngle();
        _pSptPod->setPos(x, -y);
        _pSptPod->setRotate(r);
    }
    
    _pSptPod->draw();
    vDraw();
}

void Target::setPos(float x, float y){
    _pSptPod->setPos(x, y);
    float r;
    _pSptPod->getRotate(r);
    _pBody->SetTransform(b2Vec2(x*B2_SCALE, -y*B2_SCALE), r);
}

void Target::setRotate(float r){
    _pSptPod->setRotate(r);
    float x, y;
    _pSptPod->getPos(x, y);
    _pBody->SetTransform(b2Vec2(x, y), r);
}

void Target::setTransform(float posX, float posY, float rotate){
    _pSptPod->setPos(posX, posY);
    _pSptPod->setRotate(rotate);
    _pBody->SetTransform(b2Vec2(posX*B2_SCALE, -posY*B2_SCALE), rotate);
}
