#ifndef __TARGET_H__
#define __TARGET_H__

class SpritePod;

class Target{
public:
    Target(const char *file, const char *nodeName, b2World* pWorld, b2BodyDef &bodyDef, b2FixtureDef &fixtureDef);
    virtual ~Target();
    virtual void draw();
    void setPos(float x, float y);
    void setRotate(float r);
    void setTransform(float posX, float posY, float rotate);
    
private:
    virtual void vDraw(){};
    SpritePod *_pSptPod;
    b2World* _pWorld;
    b2Body* _pBody;
};

#endif //__TARGET_H__