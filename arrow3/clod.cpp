#include "stdafx.h"
#include "clod.h"
#include "lwtf/lwCamera.h"

Clod::Clod(int xsize, int ysize, int zsise){
    _xsize = xsize;
    _ysize = ysize;
    _zsize = zsise;
    
    _pCam = new lw::Camera();
    _pCam->lookat(5, 5, 5, 0, 0, 0, 0, 1, 0);
    _pCam->perspective(M_PI/3.f, 2.0f/3.0f, 1, 1000);
    
    _pShaderProg = lw::ShaderProgramRes::create("clod.vsh", "clod.fsh");
    _posLocation = _pShaderProg->getAttribLocation("position");
    _normalLocation = _pShaderProg->getAttribLocation("normal");
    _mvpMatLocation = _pShaderProg->getUniformLocation("modelViewProjectionMatrix");
    _normalMatLocation =  _pShaderProg->getUniformLocation("normalMatrix");
}

Clod::~Clod(){
    delete _pCam;
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

