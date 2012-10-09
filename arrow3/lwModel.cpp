#include "stdafx.h"
#include "lwModel.h"
#include "PVRT/OGLES2/PVRTTextureAPI.h"

namespace {
    struct TextureInfo{
        std::string file;
        GLuint glname;
        int ref;
    };
    GLuint createTexture(const char *pvrfile);
    void releaseTexture(GLuint glname);
    std::vector<TextureInfo> _textures;
    
    GLuint createTexture(const char *pvrfile){
        std::vector<TextureInfo>::iterator it = _textures.begin();
        std::vector<TextureInfo>::iterator itend = _textures.end();
        for ( ; it != itend; ++it ){
            if ( it->file.compare(pvrfile) == 0 ){
                ++it->ref;
                return it->glname;
            }
        }
        TextureInfo texinfo;
        texinfo.ref = 1;
        texinfo.file = pvrfile;
        EPVRTError err = PVRTTextureLoadFromPVR(_f(pvrfile), &texinfo.glname);
        if ( err ){
            lwerror("PVRTTextureLoadFromPVR failed");
            return -1;
        }
        _textures.push_back(texinfo);
        return texinfo.glname;
    }
    void releaseTexture(GLuint glname){
        std::vector<TextureInfo>::iterator it = _textures.begin();
        std::vector<TextureInfo>::iterator itend = _textures.end();
        for ( ; it != itend; ++it ){
            if ( it->glname == glname ){
                --it->ref;
                if ( it->ref == 0 ){
                    _textures.erase(it);
                }
                break;
            }
        }
    }
}

//==========================================
LwInput::LwInput(GLint location)
:_location(location){
    
}

//--------------------------------------
class LwInputFloat : public LwInput{
public:
    LwInputFloat(GLint location, float f);
    virtual void use();
private:
    float _value;
};

LwInputFloat::LwInputFloat(GLint location, float f)
:LwInput(location), _value(f){
}

void LwInputFloat::use(){
    glUniform1f(_location, _value);
}

//--------------------------------------
class LwInputVec3 : public LwInput{
public:
    LwInputVec3(GLint location, float x, float y, float z);
    virtual void use();
private:
    cml::Vector3 _value;
};

LwInputVec3::LwInputVec3(GLint location, float x, float y, float z)
:LwInput(location), _value(x, y, z){
}

void LwInputVec3::use(){
    glUniform3f(_location, _value[0], _value[1], _value[2]);
}

//--------------------------------------
class LwInputVec4 : public LwInput{
public:
    LwInputVec4(GLint location, float x, float y, float z, float w);
    virtual void use();
private:
    cml::Vector4 _value;
};

LwInputVec4::LwInputVec4(GLint location, float x, float y, float z, float w)
:LwInput(location), _value(x, y, z, w){
}

void LwInputVec4::use(){
    glUniform4f(_location, _value[0], _value[1], _value[2], _value[3]);
}

//--------------------------------------
class LwInputTexture : public LwInput{
public:
    LwInputTexture(GLint location, const char *file, GLint unit);
    ~LwInputTexture();
    virtual void use();
private:
    GLuint _texname;
    GLint _unit;
};

LwInputTexture::LwInputTexture(GLint location, const char *file, GLint unit)
:LwInput(location), _unit(unit){
    _texname = createTexture(file);
    lwassert(_texname != -1);
}

LwInputTexture::~LwInputTexture(){
    releaseTexture(_texname);
}

void LwInputTexture::use(){
    glActiveTexture(GL_TEXTURE0+_unit);
    glBindTexture(GL_TEXTURE_2D, _texname);
    glUniform1i(_location, _unit);
}

//--------------------------------------
class LwInputPosition : public LwInput{
public:
    LwInputPosition(GLint location, SPODMesh* pMesh);
    virtual void use();
    virtual void unuse();
private:
    SPODMesh* _pMesh;
};

LwInputPosition::LwInputPosition(GLint location, SPODMesh* pMesh)
:LwInput(location), _pMesh(pMesh){
    
}

void LwInputPosition::use(){
    glEnableVertexAttribArray(_location);
	glVertexAttribPointer(_location, 3, GL_FLOAT, GL_FALSE, _pMesh->sVertex.nStride, _pMesh->sVertex.pData);
}

void LwInputPosition::unuse(){
    glDisableVertexAttribArray(_location);
}

//--------------------------------------
class LwInputNormal : public LwInput{
public:
    LwInputNormal(GLint location, SPODMesh* pMesh);
    virtual void use();
    virtual void unuse();
private:
    SPODMesh* _pMesh;
};

LwInputNormal::LwInputNormal(GLint location, SPODMesh* pMesh)
:LwInput(location), _pMesh(pMesh){
    
}

void LwInputNormal::use(){
    glEnableVertexAttribArray(_location);
	glVertexAttribPointer(_location, 3, GL_FLOAT, GL_FALSE, _pMesh->sNormals.nStride, _pMesh->sNormals.pData);
}

void LwInputNormal::unuse(){
    glDisableVertexAttribArray(_location);
}

//--------------------------------------
class LwInputBinormal : public LwInput{
public:
    LwInputBinormal(GLint location, SPODMesh* pMesh);
    virtual void use();
    virtual void unuse();
private:
    SPODMesh* _pMesh;
};

LwInputBinormal::LwInputBinormal(GLint location, SPODMesh* pMesh)
:LwInput(location), _pMesh(pMesh){
    lwassert(_pMesh->sBinormals.pData);
}

void LwInputBinormal::use(){
    glEnableVertexAttribArray(_location);
	glVertexAttribPointer(_location, 3, GL_FLOAT, GL_FALSE, _pMesh->sBinormals.nStride, _pMesh->sBinormals.pData);
}

void LwInputBinormal::unuse(){
    glDisableVertexAttribArray(_location);
}

//--------------------------------------
class LwInputTangent : public LwInput{
public:
    LwInputTangent(GLint location, SPODMesh* pMesh);
    virtual void use();
    virtual void unuse();
private:
    SPODMesh* _pMesh;
};

LwInputTangent::LwInputTangent(GLint location, SPODMesh* pMesh)
:LwInput(location), _pMesh(pMesh){
    lwassert(_pMesh->sTangents.pData);
}

void LwInputTangent::use(){
    glEnableVertexAttribArray(_location);
	glVertexAttribPointer(_location, 3, GL_FLOAT, GL_FALSE, _pMesh->sTangents.nStride, _pMesh->sTangents.pData);
}

void LwInputTangent::unuse(){
    glDisableVertexAttribArray(_location);
}

//--------------------------------------
class LwInputUV : public LwInput{
public:
    LwInputUV(GLint location, SPODMesh* pMesh, int uvIdx);
    virtual void use();
    virtual void unuse();
private:
    SPODMesh* _pMesh;
    int _uvIdx;
};

LwInputUV::LwInputUV(GLint location, SPODMesh* pMesh, int uvIdx)
:LwInput(location), _pMesh(pMesh), _uvIdx(uvIdx){
    lwassert(uvIdx>=0 && uvIdx < pMesh->nNumUVW);
}

void LwInputUV::use(){
    glEnableVertexAttribArray(_location);
	glVertexAttribPointer(_location, 2, GL_FLOAT, GL_FALSE, _pMesh->psUVW[_uvIdx].nStride, _pMesh->psUVW[_uvIdx].pData);
}

void LwInputUV::unuse(){
    glDisableVertexAttribArray(_location);
}

//--------------------------------------
class LwInputWV : public LwInput{
public:
    LwInputWV(GLint location, LwMesh* pMesh);
    virtual void use();
private:
    LwMesh* _pMesh;
};

LwInputWV::LwInputWV(GLint location, LwMesh* pMesh)
:LwInput(location), _pMesh(pMesh){
    
}

void LwInputWV::use(){
    lw::Camera* pCam = _pMesh->getCamera();
    cml::Matrix4 mView;
    pCam->getView(mView);
    PVRTMat4 world = *(_pMesh->getWorldMatrix());
    PVRTMat4 view(mView.data());
    
    PVRTMat4 wv = view * world;
    glUniformMatrix4fv(_location, 1, GL_FALSE, wv.f);
}

//--------------------------------------
class LwInputWVP : public LwInput{
public:
    LwInputWVP(GLint location, LwMesh* pMesh);
    virtual void use();
private:
    LwMesh* _pMesh;
};

LwInputWVP::LwInputWVP(GLint location, LwMesh* pMesh)
:LwInput(location), _pMesh(pMesh){
    
}

void LwInputWVP::use(){
    lw::Camera* pCam = _pMesh->getCamera();
    cml::Matrix4 mViewProj;
    pCam->getViewProj(mViewProj);
    PVRTMat4 world = *(_pMesh->getWorldMatrix());
    PVRTMat4 viewProj(mViewProj.data());
    
    PVRTMat4 wvp = viewProj * world;
    glUniformMatrix4fv(_location, 1, GL_FALSE, wvp.f);
}

//--------------------------------------
class LwInputWVIT : public LwInput{
public:
    LwInputWVIT(GLint location, LwMesh* pMesh);
    virtual void use();
private:
    LwMesh* _pMesh;
};

LwInputWVIT::LwInputWVIT(GLint location, LwMesh* pMesh)
:LwInput(location), _pMesh(pMesh){
    
}

void LwInputWVIT::use(){
    lw::Camera* pCam = _pMesh->getCamera();
    cml::Matrix4 mView;
    pCam->getView(mView);
    PVRTMat4 world = *(_pMesh->getWorldMatrix());
    PVRTMat4 view(mView.data());
    
    PVRTMat4 wv = view * world;
    PVRTMat3 wvit(wv.inverse().transpose());
    
    glUniformMatrix3fv(_location, 1, GL_FALSE, wvit.f);
}

//==========================================
LwMesh::LwMesh(const tinyxml2::XMLElement* pElemMesh, CPVRTModelPOD& pod)
:_textureUnit(0), _pCamera(NULL), _pmWorld(NULL){
    _meshId = pElemMesh->IntAttribute("id");
    lwassert(_meshId < pod.nNumMesh );
    _pMesh = pod.pMesh + _meshId;
    
    const char *lwfx = pElemMesh->Attribute("effects");
    _pEffects = lw::EffectsRes::create(lwfx);
    lwassert(_pEffects);
    const std::vector<lw::EffectsRes::LocSmt> &locSmts = _pEffects->getLocSmts();
    std::vector<lw::EffectsRes::LocSmt>::const_iterator it = locSmts.begin();
    std::vector<lw::EffectsRes::LocSmt>::const_iterator itend = locSmts.end();
    for ( ; it != itend; ++it ){
        loadSemantic(*it, _pMesh);
    }
    
    const tinyxml2::XMLElement *pElemInput = pElemMesh->FirstChildElement("input");
    while ( pElemInput ) {
        const char *name = pElemInput->Attribute("name");
        const char *type = pElemInput->Attribute("type");
        const char *value = pElemInput->Attribute("value");
        lwassert(name && type && value);
        loadInput(name, type, value);
        pElemInput = pElemInput->NextSiblingElement("input");
    }
    pElemMesh = pElemMesh->NextSiblingElement("mesh");
}

LwMesh::~LwMesh(){
    _pEffects->release();
    std::vector<LwInput*>::iterator it = _inputs.begin();
    std::vector<LwInput*>::iterator itend = _inputs.end();
    for ( ; it != itend; ++it ){
        delete (*it);
    }
}

void LwMesh::draw(lw::Camera* pCamera, const PVRTMat4* pmWorld){
    _pCamera = pCamera;
    _pmWorld = pmWorld;
    _pEffects->use();
    std::vector<LwInput*>::iterator it = _inputs.begin();
    std::vector<LwInput*>::iterator itend = _inputs.end();
    for ( ; it != itend; ++it ){
        (*it)->use();
    }
    
    //todo
    if(_pMesh->nNumStrips == 0)
	{
		glDrawElements(GL_TRIANGLES, _pMesh->nNumFaces*3, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		int offset = 0;
		for(int i = 0; i < (int)_pMesh->nNumStrips; ++i)
		{
			glDrawElements(GL_TRIANGLE_STRIP, _pMesh->pnStripLength[i]+2, GL_UNSIGNED_SHORT, &((GLshort*)0)[offset]);
            offset += _pMesh->pnStripLength[i]+2;
		}
	}
    
    it = _inputs.begin();
    for ( ; it != itend; ++it ){
        (*it)->unuse();
    }
}

lw::Camera* LwMesh::getCamera(){
    return _pCamera;
}

const PVRTMat4* LwMesh::getWorldMatrix(){
    return _pmWorld;
}

void LwMesh::loadSemantic(const lw::EffectsRes::LocSmt& locSmt, SPODMesh* pMesh){
    if ( locSmt.semantic == lw::EffectsRes::POSITION ){
        LwInputPosition* pInput = new LwInputPosition(locSmt.location, pMesh);
        _inputs.push_back(pInput);
    }else if ( locSmt.semantic == lw::EffectsRes::NORMAL ){
        LwInputNormal* pInput = new LwInputNormal(locSmt.location, pMesh);
        _inputs.push_back(pInput);
    }else if ( locSmt.semantic == lw::EffectsRes::BINORMAL ){
        LwInputBinormal* pInput = new LwInputBinormal(locSmt.location, pMesh);
        _inputs.push_back(pInput);
    }else if ( locSmt.semantic == lw::EffectsRes::TANGENT ){
        LwInputTangent* pInput = new LwInputTangent(locSmt.location, pMesh);
        _inputs.push_back(pInput);
    }else if ( locSmt.semantic >= lw::EffectsRes::UV0 && locSmt.semantic <= lw::EffectsRes::UV3 ){
        LwInputUV* pInput = new LwInputUV(locSmt.location, pMesh, locSmt.semantic-lw::EffectsRes::UV0);
        _inputs.push_back(pInput);
    }else if ( locSmt.semantic == lw::EffectsRes::WORLDVIEW ){
        LwInputWV* pInput = new LwInputWV(locSmt.location, this);
        _inputs.push_back(pInput);
    }else if ( locSmt.semantic == lw::EffectsRes::WORLDVIEWPROJ ){
        LwInputWVP* pInput = new LwInputWVP(locSmt.location, this);
        _inputs.push_back(pInput);
    }else if ( locSmt.semantic == lw::EffectsRes::WORLDVIEWIT ){
        LwInputWVIT* pInput = new LwInputWVIT(locSmt.location, this);
        _inputs.push_back(pInput);
    }
}

void LwMesh::loadInput(const char *name, const char *type, const char *value){
    int loc = _pEffects->getUniformLocation(name);
    if ( loc == -1 ){
        lwerror("loc == -1:" << name);
    }
    if ( strcmp(type, "FLOAT") == 0 ){
        float f;
        sscanf(value, "%f", &f);
        LwInputFloat* pInput = new LwInputFloat(loc, f);
        _inputs.push_back(pInput);
    }else if ( strcmp(type, "VEC3") == 0 ){
        float x, y, z;
        sscanf(value, "%f,%f,%f", &x, &y, &z);
        LwInputVec3* pInput = new LwInputVec3(loc, x, y, z);
        _inputs.push_back(pInput);
    }else if ( strcmp(type, "VEC4") == 0 ){
        float x, y, z, w;
        sscanf(value, "%f,%f,%f,%f", &x, &y, &z, &w);
        LwInputVec4* pInput = new LwInputVec4(loc, x, y, z, w);
        _inputs.push_back(pInput);
    }else if ( strcmp(type, "TEXTURE") == 0 ){
        LwInputTexture* pInput = new LwInputTexture(loc, value, _textureUnit);
        ++_textureUnit;
        _inputs.push_back(pInput);
    }
}

//==========================================

LwModel::LwModel(const char *mdlFile){
    lwassert(mdlFile);
    
    //lwmdl file
    tinyxml2::XMLDocument doc;
    doc.LoadFile(_f(mdlFile));
    const tinyxml2::XMLElement *pElemLwmdl = doc.RootElement();
    const char *podFile = pElemLwmdl->Attribute("pod");
    lwassert(podFile);
    
    //vbos
    _vbos = _indexVbos = NULL;
    
    if(_pod.ReadFromFile(_f(podFile)) != PVR_SUCCESS){
        lwerror("Failed to load pod file. file=" << podFile);
        lwassert(0);
        return;
    }
    
    if(!_pod.pMesh[0].pInterleaved){
		lwerror("requires the pod data to be interleaved");
        lwassert(0);
		return;
	}
    
    for ( int i = 0; i < _pod.nNumMeshNode; ++i ){
        SPODNode& node = _pod.pNode[i];
        lwinfo(node.pszName << ": " << node.nIdx);
    }
    
    _vbosNum = _pod.nNumMesh;
	_vbos = new GLuint[_vbosNum];
	_indexVbos = new GLuint[_vbosNum];
    
	glGenBuffers(_pod.nNumMesh, _vbos);
	for (unsigned int i = 0; i < _pod.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = _pod.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, _vbos[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);
        
		// Load index data into buffer object if available
		_indexVbos[i] = 0;
		if (Mesh.sFaces.pData)
		{
			glGenBuffers(1, &_indexVbos[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexVbos[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    //mesh
    const tinyxml2::XMLElement *pElemMesh = pElemLwmdl->FirstChildElement("mesh");
    while ( pElemMesh ){
        LwMesh* pMesh = new LwMesh(pElemMesh, _pod);
        _meshes.push_back(pMesh);
        pElemMesh = pElemMesh->NextSiblingElement("mesh");
    }
}

LwModel::~LwModel(){
    if ( _vbos )
        glDeleteBuffers(_vbosNum, _vbos);
        delete [] _vbos;
    if ( _indexVbos )
        glDeleteBuffers(_vbosNum, _indexVbos);
        delete [] _indexVbos;
    
    std::vector<LwMesh*>::iterator it = _meshes.begin();
    std::vector<LwMesh*>::iterator itend = _meshes.end();
    for ( ; it != itend; ++it ){
        delete (*it);
    }
}

void LwModel::draw(lw::Camera *pCamera){
    for (unsigned int i = 0; i < _pod.nNumMeshNode; ++i)
	{
		SPODNode& node = _pod.pNode[i];
        
		// Get the node model matrix
		PVRTMat4 mWorld;
		mWorld = _pod.GetWorldMatrix(node);
        mWorld.postTranslate(0.f, 4.f, 0.f);
        
        int meshIdx = node.nIdx;
        if ( meshIdx < _meshes.size() ){
            lwassert(_vbos[meshIdx] && _indexVbos[meshIdx]);
            glBindBuffer(GL_ARRAY_BUFFER, _vbos[meshIdx]);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexVbos[meshIdx]);
            
            _meshes[meshIdx]->draw(pCamera, &mWorld);
            
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
	}
}
