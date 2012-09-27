#ifndef __CLOD_H__
#define __CLOD_H__

namespace lw {
    class Camera;
}

class PodModel;

class Clod{
public:
    Clod(int xsize, int ysize, int zsise);
    ~Clod();
    void draw();
    int pick(float x, float y);
    void remove(const std::vector<int>& idxs);
    bool event(const lw::TouchEvent& evt);
    
    int xyz2idx(int x, int y, int z);
    void idx2xyz(int idx, int &x, int &y, int &z);
    
private:
    int _xsize, _ysize, _zsize;
    float _offsetX, _offsetY, _offsetZ;
    lw::Camera *_pCam;
    lw::EffectsRes *_pEffect;
    int _posLocation;
    int _mvpMatLocation;
    char* _blocks;
    std::set<int> _shellBlocks;
    std::vector<float> _vertices;
    float _camRotY, _camRotX;
    float _camDist;
    std::vector<const lw::TouchEvent*> _evts;
    bool _updateCam;
    float _touchDist;
    PodModel* _pModel;
    
    void setBlocks(int x, int y, int z, bool b);
    bool getBlocks(int x, int y, int z);
    
    void updateFaces();
    void updateCam();
};


#endif //__CLOD_H__