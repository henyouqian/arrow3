#ifndef __CLOD_H__
#define __CLOD_H__

namespace lw {
    class Camera;
}

class Clod{
public:
    Clod(int xsize, int ysize, int zsise);
    ~Clod();
    void draw();
    
private:
    int _xsize, _ysize, _zsize;
    lw::Camera *_pCam;
    lw::ShaderProgramRes* _pShaderProg;
    int _posLocation;
    int _normalLocation;
    int _mvpMatLocation;
    int _normalMatLocation;
    char* _blocks;
    
    void setBlocks(int x, int y, int z, bool b);
    bool getBlocks(int x, int y, int z);
};


#endif //__CLOD_H__