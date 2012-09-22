#include "stdafx.h"
#include "clod.h"
#include "lwtf/lwCamera.h"
#include "podModel.h"

namespace{
    GLfloat g_x[] = {
        1.f, 0.f, 0.f,
        1.f, 1.f, 0.f,
        1.f, 0.f, 1.f,
        1.f, 0.f, 1.f,
        1.f, 1.f, 0.f,
        1.f, 1.f, 1.f,
    };
    GLfloat g_x_[] = {
        0.f, 1.f, 0.f,
        0.f, 0.f, 0.f,
        0.f, 1.f, 1.f,
        0.f, 1.f, 1.f,
        0.f, 0.f, 0.f,
        0.f, 0.f, 1.f,
    };
    GLfloat g_y[] = {
        1.f, 1.f, 0.f,
        0.f, 1.f, 0.f,
        1.f, 1.f, 1.f,
        1.f, 1.f, 1.f,
        0.f, 1.f, 0.f,
        0.f, 1.f, 1.f,
    };
    GLfloat g_y_[] = {
        0.f, 0.f, 0.f,
        1.f, 0.f, 0.f,
        0.f, 0.f, 1.f,
        0.f, 0.f, 1.f,
        1.f, 0.f, 0.f,
        1.f, 0.f, 1.f,
    };
    GLfloat g_z[] = {
        1.f, 1.f, 1.f,
        0.f, 1.f, 1.f,
        1.f, 0.f, 1.f,
        1.f, 0.f, 1.f,
        0.f, 1.f, 1.f,
        0.f, 0.f, 1.f,
    };
    GLfloat g_z_[] = {
        1.f, 0.f, 0.f,
        0.f, 0.f, 0.f,
        1.f, 1.f, 0.f,
        1.f, 1.f, 0.f,
        0.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
    };
    
    struct Ray{
        cml::Vector3 origin;
        cml::Vector3 direction;
    };
    
    struct Box{
        cml::Vector3 min;
        cml::Vector3 max;
    };
    
    bool intersectRayAABox2(const Ray &ray, const Box &box, float& tnear, float& tfar)
    {
        cml::Vector3 T_1, T_2; // vectors to hold the T-values for every direction
        float t_near = -FLT_MAX; // maximums defined in float.h
        float t_far = FLT_MAX;
        
        for (int i = 0; i < 3; i++){ //we test slabs in every direction
            if (ray.direction[i] == 0){ // ray parallel to planes in this direction
                if ((ray.origin[i] < box.min[i]) || (ray.origin[i] > box.max[i])) {
                    return false; // parallel AND outside box : no intersection possible
                }
            } else { // ray not parallel to planes in this direction
                T_1[i] = (box.min[i] - ray.origin[i]) / ray.direction[i];
                T_2[i] = (box.max[i] - ray.origin[i]) / ray.direction[i];
                
                if(T_1[i] > T_2[i]){ // we want T_1 to hold values for intersection with near plane
                    std::swap(T_1,T_2);
                }
                if (T_1[i] > t_near){
                    t_near = T_1[i];
                }
                if (T_2[i] < t_far){
                    t_far = T_2[i];
                }
                if( (t_near > t_far) || (t_far < 0) ){
                    return false;
                }
            }
        }
        tnear = t_near; tfar = t_far; // put return values in place
        return true; // if we made it here, there was an intersection - YAY
    }
}

Clod::Clod(int xsize, int ysize, int zsise){
    _xsize = xsize;
    _ysize = ysize;
    _zsize = zsise;
    
    _offsetX = -_xsize * .5f;
    _offsetY = 0.f;
    _offsetZ = -_zsize * .5f;
    
    //_offsetX = _offsetY = _offsetZ = 0;
    
    _pCam = new lw::Camera();
    //_pCam->lookat(20, 20, 20, 0, 0, 0, 0, 1, 0);
    _pCam->perspective(M_PI/3.f, 2.0f/3.0f, 1, 1000);
    
    _pShaderProg = lw::ShaderProgramRes::create("clod_.vsh", "clod_.fsh");
    _posLocation = _pShaderProg->getAttribLocation("position");
    _mvpMatLocation = _pShaderProg->getUniformLocation("modelViewProjectionMatrix");
    
    int blocksNum = xsize * ysize * zsise;
    int sz = blocksNum/8+1;
    _blocks = new char[sz];
    memset(_blocks, 0xff, sz);
    
    for ( int i = 0; i < blocksNum; ++i ){
        int x, y, z;
        idx2xyz(i, x, y, z);
        if ( x == 0 || x == xsize-1 || y == 0 || y == ysize-1 || z == 0 || z == zsise-1 ){
            _shellBlocks.insert(i);
        }
    }
    
    updateFaces();
    
    _pModel = new PodModel("Scene.pod");
    
    _camRotY = _camRotX = 0.f;
    _camDist = 40.f;
    _updateCam = true;
    updateCam();
}

Clod::~Clod(){
    delete _pCam;
    delete [] _blocks;
    delete _pModel;
}

void Clod::draw(){
    if ( _updateCam ){
        updateCam();
    }
    
    cml::Matrix4 mvp;
    static float f = 0;
    f += .01f;
    cml::Matrix4 m;
    cml::matrix_rotation_world_y(m, 0.f);
    
    _pCam->getViewProj(mvp);
    //mvp = mvp * m;
    
    cml::Matrix4 mNormal4 = cml::transpose(cml::inverse(m)); 
    cml::Matrix3 mNormal(m(0,0), m(0,1), m(0,2), 
                        m(1,0), m(1,1), m(1,2), 
                        m(2,0), m(2,1), m(2,2));
    
    _pShaderProg->use();
    glUniformMatrix4fv(_mvpMatLocation, 1, false, mvp.data());
    
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
    const char* p = (char*)(&_vertices[0]);
    glEnableVertexAttribArray(_posLocation);
    glVertexAttribPointer(_posLocation, 4, GL_FLOAT, GL_FALSE, 16, p);
    
    glDrawArrays(GL_TRIANGLES, 0, _vertices.size()/4);
    glDisableVertexAttribArray(_posLocation);
    
    _pModel->draw();
}

int Clod::pick(float x, float y){
    float w, h;
    lw::App::s().getScreenSize(w, h);
    x = 2.f * x / w - 1.f;
    y = 1.f - 2.f * y / h;
    
    cml::Matrix4 m;
    cml::Vector4 v;
    _pCam->getViewProj(m);
    m = cml::inverse(m);
    v.set(x, y, 0.f, 1.f);
    v = m * v;
    v /= v[3];
    
    _pCam->getView(m);
    m = cml::inverse(m);
    float *p = m.data();
    cml::Vector4 camPos(p[12], p[13], p[14], 1.f);
    
    Ray ray;
    ray.origin.set(camPos[0], camPos[1], camPos[2]);
    cml::Vector3 vTo(v[0], v[1], v[2]);
    ray.direction = vTo - ray.origin;
    
    int hitIdx = -1;
    std::set<int>::iterator it = _shellBlocks.begin();
    std::set<int>::iterator itend = _shellBlocks.end();
    Box box;
    int ix, iy, iz;
    float fx, fy, fz;
    float near, far;
    float nearmin = FLT_MAX;
    for ( ; it != itend; ++it ){
        int idx = *it;
        idx2xyz(idx, ix, iy, iz);
        fx = ix + _offsetX;
        fy = iy + _offsetY;
        fz = iz + _offsetZ;
        box.min.set(fx, fy, fz);
        box.max.set(fx+1, fy+1, fz+1);
        if ( intersectRayAABox2(ray, box, near, far) ){
            if ( near < nearmin ){
                hitIdx = idx;
                nearmin = near;
            }
        }
    }
    if ( hitIdx >= 0 ){
        return hitIdx;
    }
    return -1;
}

void Clod::remove(const std::vector<int>& idxs){
    for ( int i = 0; i < idxs.size(); ++i ){
        int x, y, z;
        int idx = idxs[i];
        idx2xyz(idx, x, y, z);
        std::set<int>::iterator it = _shellBlocks.find(idx);
        if ( it != _shellBlocks.end() ){
            _shellBlocks.erase(it);
            setBlocks(x, y, z, false);
            if ( x != _xsize-1 && getBlocks(x+1, y, z)){
                _shellBlocks.insert(xyz2idx(x+1, y, z));
            }
            if ( x != 0 && getBlocks(x-1, y, z)){
                _shellBlocks.insert(xyz2idx(x-1, y, z));
            }
            if ( y != _ysize-1 && getBlocks(x, y+1, z)){
                _shellBlocks.insert(xyz2idx(x, y+1, z));
            }
            if ( y != 0 && getBlocks(x, y-1, z)){
                _shellBlocks.insert(xyz2idx(x, y-1, z));
            }
            if ( z != _zsize-1 && getBlocks(x, y, z+1)){
                _shellBlocks.insert(xyz2idx(x, y, z+1));
            }
            if ( z != 0 && getBlocks(x, y, z-1)){
                _shellBlocks.insert(xyz2idx(x, y, z-1));
            }
        }
    }
    
    updateFaces();
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

int Clod::xyz2idx(int x, int y, int z){
    return _xsize*_zsize*y+_xsize*z+x;
}

void Clod::idx2xyz(int idx, int &x, int &y, int &z){
    y = idx / (_xsize*_zsize);
    int remain = idx % (_xsize*_zsize);
    z = remain/_xsize;
    x = remain%_xsize;
}

namespace{
    void addSquare(std::vector<float> &verts, float *p, float x, float y, float z, float bright){
        for ( int row = 0; row < 6; ++row ){
            for ( int col = 0; col < 4; ++col ){
                int idx = row*3+col;
                if ( col == 0 ){
                    verts.push_back(p[idx]+x);
                }else if ( col == 1 ){
                    verts.push_back(p[idx]+y);
                }else if ( col == 2 ){
                    verts.push_back(p[idx]+z);
                }else{
                    verts.push_back(bright);
                }
            }
        }
    }
}

void Clod::updateFaces(){
    std::set<int>::iterator it = _shellBlocks.begin();
    std::set<int>::iterator itend = _shellBlocks.end();
    int x, y, z;
    _vertices.clear();
    for ( ; it != itend; ++it ){
        idx2xyz(*it, x, y, z);
        float posX = x + _offsetX;
        float posY = y + _offsetY;
        float posZ = z + _offsetZ;
        if ( x == _xsize-1 || !getBlocks(x+1, y, z)){
            addSquare(_vertices, g_x, posX, posY, posZ, .5);
        }
        if ( x == 0 || !getBlocks(x-1, y, z)){
            addSquare(_vertices, g_x_, posX, posY, posZ, .8f);
        }
        if ( y == _ysize-1 || !getBlocks(x, y+1, z)){
            addSquare(_vertices, g_y, posX, y, posZ, 1.f);
        }
        if ( y == 0 || !getBlocks(x, y-1, z)){
            addSquare(_vertices, g_y_, posX, posY, posZ, .6f);
        }
        if ( z == _zsize-1 || !getBlocks(x, y, z+1)){
            addSquare(_vertices, g_z, posX, posY, posZ, .9f);
        }
        if ( z == 0 || !getBlocks(x, y, z-1)){
            addSquare(_vertices, g_z_, posX, posY, posZ, .7f);
        }
    }
}

void Clod::updateCam(){
    _updateCam = false;
    if ( _evts.size() == 2 ){
        cml::Vector2 v1(_evts[0]->x, _evts[0]->y);
        cml::Vector2 v2(_evts[1]->x, _evts[1]->y);
        cml::Vector2 dv = v1 - v2;
        float dist = dv.length();
        float d = dist - _touchDist;
        _camDist -= d*.001f*_camDist;
        _touchDist = dist;
    }
    
    float aimx = 0.f;
    float aimy = .5f * _ysize;
    float aimz = 0.f;
    cml::Vector4 v(0.f, 0.f, _camDist, 1.f);
    cml::Matrix4 m, m1;
    cml::matrix_rotation_world_y(m, _camRotY);
    cml::matrix_rotation_world_x(m1, _camRotX);
    m = m1 * m;
    v = v * m;
    _pCam->lookat(v[0]+aimx, v[1]+aimy, v[2]+aimz, aimx, aimy, aimz, 0.f, 1.f, 0.f);
    
    _pCam->getViewProj(m);
    _pModel->setViewProj(m);
}

bool Clod::event(const lw::TouchEvent& evt){
    if ( evt.type == lw::TouchEvent::TOUCH ){
        int idx = pick(evt.x, evt.y);
        if ( idx >= 0 ){
            std::vector<int> idxs;
            idxs.push_back(idx);
            remove(idxs);
        }
        if ( _evts.size() < 2 ){
            _evts.push_back(&evt);
            if ( _evts.size() == 2 ){
                cml::Vector2 v1(_evts[0]->x, _evts[0]->y);
                cml::Vector2 v2(_evts[1]->x, _evts[1]->y);
                cml::Vector2 dv = v1 - v2;
                _touchDist = dv.length();
            }
        }
    }else if ( evt.type == lw::TouchEvent::MOVE ){
        if ( _evts.size() == 2 && (&evt == _evts[0] || &evt == _evts[1]) ){
            _updateCam = true;
            const float f = 0.002f;
            _camRotY += (evt.x - evt.prx)*f;
            _camRotX += (evt.y - evt.pry)*f;
            float lim = (float)M_PI*.5f*.95f;
            _camRotX = cml::clamp(_camRotX, -lim, lim);
        }
    }else if ( evt.type == lw::TouchEvent::UNTOUCH ){
        std::vector<const lw::TouchEvent*>::iterator it = _evts.begin();
        std::vector<const lw::TouchEvent*>::iterator itend = _evts.end();
        for ( ; it != itend; ++it ){
            if ( *it == &evt ){
                _evts.erase(it);
                break;
            }
        }
    }
    return false;
}

