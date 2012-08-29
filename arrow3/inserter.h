#ifndef __INSERTER_H__
#define __INSERTER_H__

class Inserter{
public:
    Inserter();
    virtual ~Inserter();
    
    void main();
    void draw();
    
protected:
    virtual void vMain(){};
    virtual void vDraw(){};
    bool _hit;
};

class RayInserter : public Inserter{
public:
    RayInserter(const char *file, float u, float v, float w, float h, float ancX, float ancY);
    virtual ~RayInserter();
    void setTransform(float posX, float posY, float rotate);
    void setSpeed(float speed);
    
private:
    virtual void vMain();
    virtual void vDraw();
    
    float _speed;
    lw::Sprite *_pSpt;
};



#endif //__INSERTER_H__