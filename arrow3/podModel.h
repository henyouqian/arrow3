#ifndef __POD_MODEL_H__
#define __POD_MODEL_H__

#include "PVRT/PVRTModelPOD.h"

class PodModel{
public:
    PodModel(const char *podFile);
    ~PodModel();
    void draw();
    
    void setViewProj(const cml::Matrix4& mat);
    
private:
    CPVRTModelPOD _pod;
    GLuint* _vbos;
    GLuint* _indexVbos;
    GLuint* m_puiTextureIDs;
    int _vbosNum;
    
    lw::EffectsRes* _pEffects;
    
    int _posLoc;
    int _uvLoc;
    int _mvpLoc;
    int _samplerLoc;
    int _colorLoc;
    
    PVRTMat4 _viewProjMat;
    
    void drawMesh(int i32NodeIndex);
};


#endif //__POD_MODEL_H__