#ifndef __LW_MODEL_H__
#define __LW_MODEL_H__

#include "PVRT/PVRTModelPOD.h"
#include "tinyxml2/tinyxml2.h"

class LwInput{
public:
    LwInput(GLint location);
    virtual ~LwInput(){};
    virtual void use() = 0;
    virtual void unuse(){};
    
protected:
    GLint _location;
};

class LwMesh{
public:
    LwMesh(const tinyxml2::XMLElement* pElemMesh, CPVRTModelPOD& pod);
    ~LwMesh();
    
private:
    int _meshId;
    SPODMesh* _pMesh;
    lw::EffectsRes* _pEffects;
    std::vector<LwInput*> _inputs;
    int _textureUnit;
    
    void loadSemantic(const lw::EffectsRes::LocSmt& locSmt, SPODMesh* pMesh);
    void loadInput(const char *name, const char *type, const char *value);
};

class LwModel{
public:
    LwModel(const char *mdlFile);
    ~LwModel();
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
    
    PVRTMat4 _viewProjMat;
    std::vector<LwMesh*> _meshes;
    
    void drawMesh(int i32NodeIndex);
};


#endif //__LW_MODEL_H__