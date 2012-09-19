#include "stdafx.h"
#include "clod.h"
#include "lwtf/lwCamera.h"

namespace{
    GLfloat g_x[] = {
        1.f, 0.f, 0.f,        1.0f, 0.0f, 0.0f,
        1.f, 1.f, 0.f,         1.0f, 0.0f, 0.0f,
        1.f, 0.f, 1.f,         1.0f, 0.0f, 0.0f,
        1.f, 0.f, 1.f,         1.0f, 0.0f, 0.0f,
        1.f, 1.f, 0.f,         1.0f, 0.0f, 0.0f,
        1.f, 1.f, 1.f,          1.0f, 0.0f, 0.0f,
    };
    GLfloat g_x_[] = {
        0.f, 1.f, 0.f,        -1.0f, 0.0f, 0.0f,
        0.f, 0.f, 0.f,       -1.0f, 0.0f, 0.0f,
        0.f, 1.f, 1.f,         -1.0f, 0.0f, 0.0f,
        0.f, 1.f, 1.f,         -1.0f, 0.0f, 0.0f,
        0.f, 0.f, 0.f,       -1.0f, 0.0f, 0.0f,
        0.f, 0.f, 1.f,        -1.0f, 0.0f, 0.0f,
    };
    GLfloat g_y[] = {
        1.f, 1.f, 0.f,         0.0f, 1.0f, 0.0f,
        0.f, 1.f, 0.f,        0.0f, 1.0f, 0.0f,
        1.f, 1.f, 1.f,          0.0f, 1.0f, 0.0f,
        1.f, 1.f, 1.f,          0.0f, 1.0f, 0.0f,
        0.f, 1.f, 0.f,        0.0f, 1.0f, 0.0f,
        0.f, 1.f, 1.f,         0.0f, 1.0f, 0.0f,
    };
    GLfloat g_y_[] = {
        0.f, 0.f, 0.f,       0.0f, -1.0f, 0.0f,
        1.f, 0.f, 0.f,        0.0f, -1.0f, 0.0f,
        0.f, 0.f, 1.f,        0.0f, -1.0f, 0.0f,
        0.f, 0.f, 1.f,        0.0f, -1.0f, 0.0f,
        1.f, 0.f, 0.f,        0.0f, -1.0f, 0.0f,
        1.f, 0.f, 1.f,         0.0f, -1.0f, 0.0f,
    };
    GLfloat g_z[] = {
        1.f, 1.f, 1.f,          0.0f, 0.0f, 1.0f,
        0.f, 1.f, 1.f,         0.0f, 0.0f, 1.0f,
        1.f, 0.f, 1.f,         0.0f, 0.0f, 1.0f,
        1.f, 0.f, 1.f,         0.0f, 0.0f, 1.0f,
        0.f, 1.f, 1.f,         0.0f, 0.0f, 1.0f,
        0.f, 0.f, 1.f,        0.0f, 0.0f, 1.0f,
    };
    GLfloat g_z_[] = {
        1.f, 0.f, 0.f,        0.0f, 0.0f, -1.0f,
        0.f, 0.f, 0.f,       0.0f, 0.0f, -1.0f,
        1.f, 1.f, 0.f,         0.0f, 0.0f, -1.0f,
        1.f, 1.f, 0.f,         0.0f, 0.0f, -1.0f,
        0.f, 0.f, 0.f,       0.0f, 0.0f, -1.0f,
        0.f, 1.f, 0.f,        0.0f, 0.0f, -1.0f
    };
    
    const int VERTICES_BUF_SIZE = 36 * 10000;
    float g_vertices_BUF[VERTICES_BUF_SIZE];
    float vertice_num = 0;
}

Clod::Clod(int xsize, int ysize, int zsise){
    _xsize = xsize;
    _ysize = ysize;
    _zsize = zsise;
    
    _pCam = new lw::Camera();
    _pCam->lookat(5, 5, 5, 0, 0, 0, 0, 1, 0);
    _pCam->perspective(M_PI/3.f, 2.0f/3.0f, 1, 1000);
    
    _pShaderProg = lw::ShaderProgramRes::create("clod_.vsh", "clod_.fsh");
    _posLocation = _pShaderProg->getAttribLocation("position");
    _normalLocation = _pShaderProg->getAttribLocation("normal");
    _mvpMatLocation = _pShaderProg->getUniformLocation("modelViewProjectionMatrix");
    _normalMatLocation =  _pShaderProg->getUniformLocation("normalMatrix");
    
    int blocksNum = xsize * ysize * zsise;
    int sz = blocksNum/8+1;
    _blocks = new char[sz];
    memset(_blocks, 0xff, sz);
    
    setBlocks(1, 2, 3, false);
    
    bool b = getBlocks(1, 2, 3);
    b = getBlocks(2, 2, 2);
    b = getBlocks(3, 3, 3);
    setBlocks(1, 2, 3, true);
    b = getBlocks(1, 2, 3);
    setBlocks(3, 3, 3, false);
    b = getBlocks(3, 3, 3);
}

Clod::~Clod(){
    delete _pCam;
    delete [] _blocks;
}

void Clod::draw(){
    glClearColor(1, 1, 1, 1.0f);
    //glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    cml::Matrix4 mvp;
    static float f = 0;
    f += .01f;
    cml::Matrix4 m;
    cml::matrix_rotation_world_x(m, 0.f);
    
    _pCam->getViewProj(mvp);
    //mvp = mvp * m;
    
    cml::Matrix4 mNormal4 = cml::transpose(cml::inverse(m)); 
    cml::Matrix3 mNormal(m(0,0), m(0,1), m(0,2), 
                        m(1,0), m(1,1), m(1,2), 
                        m(2,0), m(2,1), m(2,2));
    
    glUniformMatrix3fv(_normalMatLocation, 1, false, mNormal.data());
    glUniformMatrix4fv(_mvpMatLocation, 1, false, mvp.data());
    
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
    GLfloat gCubeVertexData[216] = 
    {
        // Data layout for each line below is:
        // positionX, positionY, positionZ,     normalX, normalY, normalZ,
        1.f, 0.f, 0.f,        1.0f, 0.0f, 0.0f,
        1.f, 1.f, 0.f,         1.0f, 0.0f, 0.0f,
        1.f, 0.f, 1.f,         1.0f, 0.0f, 0.0f,
        1.f, 0.f, 1.f,         1.0f, 0.0f, 0.0f,
        1.f, 1.f, 0.f,         1.0f, 0.0f, 0.0f,
        1.f, 1.f, 1.f,          1.0f, 0.0f, 0.0f,
        
        1.f, 1.f, 0.f,         0.0f, 1.0f, 0.0f,
        0.f, 1.f, 0.f,        0.0f, 1.0f, 0.0f,
        1.f, 1.f, 1.f,          0.0f, 1.0f, 0.0f,
        1.f, 1.f, 1.f,          0.0f, 1.0f, 0.0f,
        0.f, 1.f, 0.f,        0.0f, 1.0f, 0.0f,
        0.f, 1.f, 1.f,         0.0f, 1.0f, 0.0f,
        
        0.f, 1.f, 0.f,        -1.0f, 0.0f, 0.0f,
        0.f, 0.f, 0.f,       -1.0f, 0.0f, 0.0f,
        0.f, 1.f, 1.f,         -1.0f, 0.0f, 0.0f,
        0.f, 1.f, 1.f,         -1.0f, 0.0f, 0.0f,
        0.f, 0.f, 0.f,       -1.0f, 0.0f, 0.0f,
        0.f, 0.f, 1.f,        -1.0f, 0.0f, 0.0f,
        
        0.f, 0.f, 0.f,       0.0f, -1.0f, 0.0f,
        1.f, 0.f, 0.f,        0.0f, -1.0f, 0.0f,
        0.f, 0.f, 1.f,        0.0f, -1.0f, 0.0f,
        0.f, 0.f, 1.f,        0.0f, -1.0f, 0.0f,
        1.f, 0.f, 0.f,        0.0f, -1.0f, 0.0f,
        1.f, 0.f, 1.f,         0.0f, -1.0f, 0.0f,
        
        1.f, 1.f, 1.f,          0.0f, 0.0f, 1.0f,
        0.f, 1.f, 1.f,         0.0f, 0.0f, 1.0f,
        1.f, 0.f, 1.f,         0.0f, 0.0f, 1.0f,
        1.f, 0.f, 1.f,         0.0f, 0.0f, 1.0f,
        0.f, 1.f, 1.f,         0.0f, 0.0f, 1.0f,
        0.f, 0.f, 1.f,        0.0f, 0.0f, 1.0f,
        
        1.f, 0.f, 0.f,        0.0f, 0.0f, -1.0f,
        0.f, 0.f, 0.f,       0.0f, 0.0f, -1.0f,
        1.f, 1.f, 0.f,         0.0f, 0.0f, -1.0f,
        1.f, 1.f, 0.f,         0.0f, 0.0f, -1.0f,
        0.f, 0.f, 0.f,       0.0f, 0.0f, -1.0f,
        0.f, 1.f, 0.f,        0.0f, 0.0f, -1.0f
    };
    
    const char* p = (char*)gCubeVertexData;
    glEnableVertexAttribArray(_posLocation);
    glVertexAttribPointer(_posLocation, 3, GL_FLOAT, GL_FALSE, 24, p);
    glEnableVertexAttribArray(_normalLocation);
    glVertexAttribPointer(_normalLocation, 3, GL_FLOAT, GL_FALSE, 24, p+12);
    
    _pShaderProg->use();
    
    glDrawArrays(GL_TRIANGLES, 0, sizeof(gCubeVertexData)/24);
}

void Clod::setBlocks(int x, int y, int z, bool b){
    if ( x >= _xsize || y >= _ysize || z >= _zsize ){
        lwerror("invalid idx");
        return;
    }
    int idx = _xsize*_zsize*y+_xsize*z+x;
    char& c = _blocks[idx/8];
    if ( b ){
        c |= (1 << (idx%8));
    }else{
        c &= (~(char)(1<<(idx%8)));
    }
}

bool Clod::getBlocks(int x, int y, int z){
    if ( x >= _xsize || y >= _ysize || z >= _zsize ){
        lwerror("invalid idx");
        return false;
    }
    int idx = _xsize*_zsize*y+_xsize*z+x;
    char& c = _blocks[idx/8];
    return c & ((char)(1 << (idx%8)));
}

