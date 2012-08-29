#ifndef __SPRITE_POD_H__
#define __SPRITE_POD_H__


class SpritePod{
public:
    SpritePod(const char *file, const char *nodeName);
    ~SpritePod();
    
    void draw();
    void setColor(const lw::Color &color);
    void setBlendMode(lw::BlendMode blendMode);
    void setPos(float x, float y);
    void setRotate(float r);
    void setScale(float x, float y);
    void getPos(float &x, float &y);
    void getRotate(float &r);
    void getScale(float &x, float &y);
    const std::vector<lw::SpriteVertex>& getVertices();
    
private:
    void update();
    std::vector<lw::SpriteVertex> _vertices;
    std::vector<lw::SpriteVertex> _transVertices;
    lw::TextureRes* _pTexRes;
    lw::Color _color;
    lw::BlendMode _blendMode;
    float _posX, _posY;
    float _rotate;
    float _scaleX, _scaleY;
    bool _needUpdate;
};

#endif //__SPRITE_POD_H__