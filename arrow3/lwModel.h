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
    void draw(lw::Camera* pCamera, const PVRTMat4* pmWorld);
    
    lw::Camera* getCamera();
    const PVRTMat4* getWorldMatrix();
    
private:
    int _meshId;
    SPODMesh* _pMesh;
    lw::EffectsRes* _pEffects;
    std::vector<LwInput*> _inputs;
    int _textureUnit;
    lw::Camera* _pCamera;
    const PVRTMat4* _pmWorld;
    
    void loadSemantic(const lw::EffectsRes::LocSmt& locSmt, SPODMesh* pMesh);
    void loadInput(const char *name, const char *type, const char *value);
};

class LwModel{
public:
    LwModel(const char *mdlFile);
    ~LwModel();
    void draw(lw::Camera *pCamera);
    
private:
    CPVRTModelPOD _pod;
    GLuint* _vbos;
    GLuint* _indexVbos;
    int _vbosNum;
  
    std::vector<LwMesh*> _meshes;
};


#endif //__LW_MODEL_H__