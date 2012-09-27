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
    
    _pEffect = lw::EffectsRes::create("clod.lwfx");
    _posLocation = _pEffect->getLocationFromSemantic(lw::EffectsRes::POSITION);
    _mvpMatLocation = _pEffect->getLocationFromSemantic(lw::EffectsRes::WORLDVIEWPROJ);
    
    _pFXMarchingCube = lw::EffectsRes::create("marchingCube.lwfx");
    _mcPosLoc = _pFXMarchingCube->getLocationFromSemantic(lw::EffectsRes::POSITION);
    _mcNormLoc = _pFXMarchingCube->getLocationFromSemantic(lw::EffectsRes::NORMAL);
    _mcWvpLoc = _pFXMarchingCube->getLocationFromSemantic(lw::EffectsRes::WORLDVIEWPROJ);
    
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
    _camDist = 30.f;
    _updateCam = true;
    updateCam();
    
    buildMc();
}

Clod::~Clod(){
    delete _pCam;
    delete [] _blocks;
    delete _pModel;
    _pEffect->release();
    _pFXMarchingCube->release();
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
    
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
//    _pEffect->use();
//    glUniformMatrix4fv(_mvpMatLocation, 1, false, mvp.data());
//    const char* p = (char*)(&_vertices[0]);
//    glEnableVertexAttribArray(_posLocation);
//    glVertexAttribPointer(_posLocation, 4, GL_FLOAT, GL_FALSE, 16, p);
//    //glDrawArrays(GL_TRIANGLES, 0, _vertices.size()/4);
//    glDisableVertexAttribArray(_posLocation);
    
    _pFXMarchingCube->use();
    glUniformMatrix4fv(_mcWvpLoc, 1, false, mvp.data());
    const char* p = (char*)(&_qVertices[0]);
    glEnableVertexAttribArray(_mcPosLoc);
    glVertexAttribPointer(_mcPosLoc, 3, GL_FLOAT, GL_FALSE, 24, p);
    glEnableVertexAttribArray(_mcNormLoc);
    glVertexAttribPointer(_mcNormLoc, 3, GL_FLOAT, GL_FALSE, 24, p+12);
    
    glDrawArrays(GL_TRIANGLES, 0, _qVertices.size()/6);
    glDisableVertexAttribArray(_mcPosLoc);
    glDisableVertexAttribArray(_mcNormLoc);
    
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
    if ( x >= _xsize || y >= _ysize || z >= _zsize || x < 0 || y < 0 || z < 0 ){
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
    
    buildMc();
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

//bool Clod::event(const lw::TouchEvent& evt){
//    if ( evt.type == lw::TouchEvent::TOUCH ){
//        int idx = pick(evt.x, evt.y);
//        if ( idx >= 0 ){
//            std::vector<int> idxs;
//            idxs.push_back(idx);
//            remove(idxs);
//        }
//        if ( _evts.size() < 2 ){
//            _evts.push_back(&evt);
//            if ( _evts.size() == 2 ){
//                cml::Vector2 v1(_evts[0]->x, _evts[0]->y);
//                cml::Vector2 v2(_evts[1]->x, _evts[1]->y);
//                cml::Vector2 dv = v1 - v2;
//                _touchDist = dv.length();
//            }
//        }
//    }else if ( evt.type == lw::TouchEvent::MOVE ){
//        if ( _evts.size() == 2 && (&evt == _evts[0] || &evt == _evts[1]) ){
//            _updateCam = true;
//            const float f = 0.002f;
//            _camRotY += (evt.x - evt.prx)*f;
//            _camRotX += (evt.y - evt.pry)*f;
//            float lim = (float)M_PI*.5f*.95f;
//            _camRotX = cml::clamp(_camRotX, -lim, lim);
//        }
//    }else if ( evt.type == lw::TouchEvent::UNTOUCH ){
//        std::vector<const lw::TouchEvent*>::iterator it = _evts.begin();
//        std::vector<const lw::TouchEvent*>::iterator itend = _evts.end();
//        for ( ; it != itend; ++it ){
//            if ( *it == &evt ){
//                _evts.erase(it);
//                break;
//            }
//        }
//    }
//    return false;
//}

bool Clod::event(const lw::TouchEvent& evt){
    if ( evt.type == lw::TouchEvent::TOUCH ){
        int idx = pick(evt.x, evt.y);
        if ( idx >= 0 ){
            std::vector<int> idxs;
            idxs.push_back(idx);
            remove(idxs);
        }
    }else if ( evt.type == lw::TouchEvent::MOVE ){
        _updateCam = true;
        const float f = 0.002f*2.f;
        _camRotY += (evt.x - evt.prx)*f;
        _camRotX += (evt.y - evt.pry)*f;
        float lim = (float)M_PI*.5f*.95f;
        _camRotX = cml::clamp(_camRotX, -lim, lim);
    }else if ( evt.type == lw::TouchEvent::UNTOUCH ){
        
    }
    return false;
}





GLint a2iTriangleConnectionTable[256][16] =  
{
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
    {3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
    {3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
    {3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
    {9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
    {2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
    {8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
    {4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
    {3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
    {1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
    {4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
    {4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
    {5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
    {2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
    {9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
    {0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
    {2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
    {10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
    {5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
    {5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
    {9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
    {1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
    {10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
    {8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
    {2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
    {7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
    {2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
    {11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
    {5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
    {11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
    {11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
    {9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
    {2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
    {6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
    {3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
    {6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
    {10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
    {6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
    {8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
    {7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
    {3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
    {0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
    {9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
    {8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
    {5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
    {0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
    {6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
    {10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
    {10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
    {8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
    {1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
    {0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
    {10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
    {3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
    {6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
    {9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
    {8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
    {3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
    {6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
    {0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
    {10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
    {10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
    {2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
    {7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
    {7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
    {2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
    {1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
    {11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
    {8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
    {0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
    {7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
    {10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
    {2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
    {6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
    {7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
    {2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
    {10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
    {10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
    {0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
    {7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
    {6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
    {8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
    {9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
    {6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
    {4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
    {10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
    {8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
    {0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
    {1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
    {8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
    {10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
    {4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
    {10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
    {11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
    {9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
    {6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
    {7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
    {3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
    {7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
    {3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
    {6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
    {9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
    {1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
    {4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
    {7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
    {6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
    {3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
    {0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
    {6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
    {0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
    {11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
    {6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
    {5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
    {9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
    {1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
    {1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
    {10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
    {0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
    {5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
    {10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
    {11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
    {9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
    {7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
    {2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
    {8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
    {9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
    {9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
    {1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
    {9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
    {5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
    {0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
    {10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
    {2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
    {0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
    {0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
    {9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
    {5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
    {3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
    {5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
    {8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
    {0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
    {9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
    {1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
    {3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
    {4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
    {9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
    {11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
    {11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
    {2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
    {9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
    {3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
    {1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
    {4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
    {3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
    {0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
    {1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};

static const int a2fVertexOffset[8][3] =
{
    {0, 0, 0},{1, 0, 0},{1, 1, 0},{0, 1, 0},
    {0, 0, 1},{1, 0, 1},{1, 1, 1},{0, 1, 1}
};

GLint aiCubeEdgeFlags[256]=
{
    0x000, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c, 0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00, 
    0x190, 0x099, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c, 0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90, 
    0x230, 0x339, 0x033, 0x13a, 0x636, 0x73f, 0x435, 0x53c, 0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30, 
    0x3a0, 0x2a9, 0x1a3, 0x0aa, 0x7a6, 0x6af, 0x5a5, 0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0, 
    0x460, 0x569, 0x663, 0x76a, 0x066, 0x16f, 0x265, 0x36c, 0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60, 
    0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0x0ff, 0x3f5, 0x2fc, 0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0, 
    0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x055, 0x15c, 0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950, 
    0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0x0cc, 0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0, 
    0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc, 0x0cc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0, 
    0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c, 0x15c, 0x055, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650, 
    0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5, 0x0ff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0, 
    0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c, 0x36c, 0x265, 0x16f, 0x066, 0x76a, 0x663, 0x569, 0x460, 
    0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac, 0x4ac, 0x5a5, 0x6af, 0x7a6, 0x0aa, 0x1a3, 0x2a9, 0x3a0, 
    0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c, 0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x033, 0x339, 0x230, 
    0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c, 0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x099, 0x190, 
    0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c, 0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x000
};

static const GLint a2iEdgeConnection[12][2] = 
{
    {0,1}, {1,2}, {2,3}, {3,0},
    {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7}
};

static const GLfloat a2fEdgeDirection[12][3] =
{
    {1.0, 0.0, 0.0},{0.0, 1.0, 0.0},{-1.0, 0.0, 0.0},{0.0, -1.0, 0.0},
    {1.0, 0.0, 0.0},{0.0, 1.0, 0.0},{-1.0, 0.0, 0.0},{0.0, -1.0, 0.0},
    {0.0, 0.0, 1.0},{0.0, 0.0, 1.0},{ 0.0, 0.0, 1.0},{0.0,  0.0, 1.0}
};

float fGetOffset(float fValue1, float fValue2, float fValueDesired)
{
    return .5f;
    float fDelta = fValue2 - fValue1;
    
    if(fDelta == 0.0f)
    {
        return 0.5f;
    }
    return (fValueDesired - fValue1)/fDelta;
}

//void vGetNormal(cml::Vector3 &rfNormal, float fX, float fY, float fZ)
//{
//    rfNormal[0] = fSample(fX-0.01, fY, fZ) - fSample(fX+0.01, fY, fZ);
//    rfNormal[1] = fSample(fX, fY-0.01, fZ) - fSample(fX, fY+0.01, fZ);
//    rfNormal[2] = fSample(fX, fY, fZ-0.01) - fSample(fX, fY, fZ+0.01);
//    rfNormal.normalize();
//}

void Clod::buildMc(){
    _qVertices.clear();
    _norms.clear();
    
    for ( int ix = -1; ix < _xsize; ++ix ){
    for ( int iy = -1; iy < _ysize; ++iy ){
    for ( int iz = -1; iz < _zsize; ++iz ){
        marchCube(ix, iy, iz);
    }}}
    
    std::map<int, cml::Vector3>::iterator it = _norms.begin();
    std::map<int, cml::Vector3>::iterator itend = _norms.end();
    for ( ; it != itend; ++it ){
        it->second.normalize();
    }
    
    VtxIdx idx;
    for ( int i = 0; i < _qVertices.size(); i += 6 ){
        idx.x = (char)(_qVertices[i]*2.f);
        idx.y = (char)(_qVertices[i+1]*2.f);
        idx.z = (char)(_qVertices[i+2]*2.f);
        std::map<int, cml::Vector3>::iterator it = _norms.find(idx.idx);
        cml::Vector3 &norm = it->second;
        _qVertices[i+3] = norm[0];
        _qVertices[i+4] = norm[1];
        _qVertices[i+5] = norm[2];
    }
}



void Clod::marchCube(int ix, int iy, int iz){
    bool afCubeValue[8];
    int iFlagIndex, iEdgeFlags, iEdge, iTriangle, iCorner, iVertex;
    float fOffset;
    cml::Vector3 asEdgeVertex[12];
    //cml::Vector3 asEdgeNorm[12];
    const float hx = _xsize*.5f;
    //const float hy = _ysize*.5f;
    const float hz = _zsize*.5f;
    
    for(int iVertex = 0; iVertex < 8; iVertex++)
    {
        afCubeValue[iVertex] = getBlocks(ix+a2fVertexOffset[iVertex][0],
                                         iy+a2fVertexOffset[iVertex][1],
                                         iz+a2fVertexOffset[iVertex][2]);
        
    }
    
    iFlagIndex = 0;
    for(int iVertexTest = 0; iVertexTest < 8; iVertexTest++)
    {
        if(!afCubeValue[iVertexTest]) 
            iFlagIndex |= 1<<iVertexTest;
    }
    
    //Find which edges are intersected by the surface
    iEdgeFlags = aiCubeEdgeFlags[iFlagIndex];
    
    //If the cube is entirely inside or outside of the surface, then there will be no intersections
    if(iEdgeFlags == 0) 
    {
        return;
    }
    
    //Find the point of intersection of the surface with each edge
    //Then find the normal to the surface at those points
    for(iEdge = 0; iEdge < 12; iEdge++)
    {
        //if there is an intersection on this edge
        if(iEdgeFlags & (1<<iEdge))
        {
            fOffset = fGetOffset(afCubeValue[ a2iEdgeConnection[iEdge][0] ], 
                                 afCubeValue[ a2iEdgeConnection[iEdge][1] ], 1.0);
            
            asEdgeVertex[iEdge][0] = ix + (a2fVertexOffset[ a2iEdgeConnection[iEdge][0] ][0]  +  fOffset * a2fEdgeDirection[iEdge][0]);
            asEdgeVertex[iEdge][1] = iy + (a2fVertexOffset[ a2iEdgeConnection[iEdge][0] ][1]  +  fOffset * a2fEdgeDirection[iEdge][1]);
            asEdgeVertex[iEdge][2] = iz + (a2fVertexOffset[ a2iEdgeConnection[iEdge][0] ][2]  +  fOffset * a2fEdgeDirection[iEdge][2]);
            
            //vGetNormal(asEdgeNorm[iEdge], asEdgeVertex[iEdge][0], asEdgeVertex[iEdge][1], asEdgeVertex[iEdge][2]);
        }
    }
    
    //Draw the triangles that were found.  There can be up to five per cube
    for(iTriangle = 0; iTriangle < 5; iTriangle++)
    {
        if(a2iTriangleConnectionTable[iFlagIndex][3*iTriangle] < 0)
            break;
        
        cml::Vector3 vtx[3];
        
        for(iCorner = 0; iCorner < 3; iCorner++)
        {
            iVertex = a2iTriangleConnectionTable[iFlagIndex][3*iTriangle+iCorner];
            
            vtx[iCorner][0] = asEdgeVertex[iVertex][0]-hx+.5f;
            vtx[iCorner][1] = asEdgeVertex[iVertex][1]+.5f;
            vtx[iCorner][2] = asEdgeVertex[iVertex][2]-hz+.5f;
            
//            _qVertices.push_back(asEdgeVertex[iVertex][0]-hx+.5f);
//            _qVertices.push_back(asEdgeVertex[iVertex][1]+.5f);
//            _qVertices.push_back(asEdgeVertex[iVertex][2]-hz+.5f);
//            _qVertices.push_back(cml::random_float(.5f, 1.f));
        }
        cml::Vector3 v1, v2, normal;
        v1 = vtx[0] - vtx[1];
        v2 = vtx[1] - vtx[2];
        normal = cml::cross(v1, v2);
        normal.normalize();
        for ( int i = 0; i < 3; ++i ){
            _qVertices.push_back(vtx[i][0]);
            _qVertices.push_back(vtx[i][1]);
            _qVertices.push_back(vtx[i][2]);
            _qVertices.push_back(normal[0]);
            _qVertices.push_back(normal[1]);
            _qVertices.push_back(normal[2]);
            VtxIdx idx;
            idx.x = (char)(vtx[i][0]*2.f);
            idx.y = (char)(vtx[i][1]*2.f);
            idx.z = (char)(vtx[i][2]*2.f);
            
            std::map<int, cml::Vector3>::iterator it = _norms.find(idx.idx);
            if ( it == _norms.end() ){
                _norms[idx.idx] = normal;
            }else{
                _norms[idx.idx] += normal;
            }
        }
        
    }
}
