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
    
    lw::EffectsRes *_pFXMarchingCube;
    int _mcPosLoc;
    int _mcNormLoc;
    int _mcWvpLoc;
    
    char* _blocks;
    std::set<int> _shellBlocks;
    std::vector<float> _vertices;
    std::vector<float> _qVertices;
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
    
    void buildMc();
    float calcAO(const cml::Vector3 &pos, const cml::Vector3 &normal);
    float aoRay(const cml::Vector3 &origin, const cml::Vector3 &dir);
    void marchCube(int ix, int iy, int iz);
    
    union VtxIdx{
        VtxIdx():idx(0){}
        struct{
            char x, y, z;
        };
        int idx;
    };
    struct NormalInfo{
        cml::Vector3 normal;
        float brightness;
    };
    std::map<int, NormalInfo> _norms;
};


#endif //__CLOD_H__