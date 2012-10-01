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
	glVertexAttribPointer(_location, 2, GL_FLOAT, GL_FALSE, _pMesh->psUVW[0].nStride, _pMesh->psUVW[0].pData);
}

void LwInputUV::unuse(){
    glDisableVertexAttribArray(_location);
}

//==========================================
LwMesh::LwMesh(const tinyxml2::XMLElement* pElemMesh, CPVRTModelPOD& pod)
:_textureUnit(0){
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

void LwMesh::loadSemantic(const lw::EffectsRes::LocSmt& locSmt, SPODMesh* pMesh){
    if ( locSmt.semantic == lw::EffectsRes::POSITION ){
        LwInputPosition* pInput = new LwInputPosition(locSmt.location, pMesh);
        _inputs.push_back(pInput);
    }else if ( locSmt.semantic == lw::EffectsRes::NORMAL ){
        LwInputNormal* pInput = new LwInputNormal(locSmt.location, pMesh);
        _inputs.push_back(pInput);
    }else if ( locSmt.semantic >= lw::EffectsRes::UV0 && locSmt.semantic <= lw::EffectsRes::UV3 ){
        LwInputUV* pInput = new LwInputUV(locSmt.location, pMesh, locSmt.semantic-lw::EffectsRes::UV0);
        _inputs.push_back(pInput);
    }
}

void LwMesh::loadInput(const char *name, const char *type, const char *value){
    int loc = _pEffects->getUniformLocation(name);
    lwassert(loc != -1);
    if ( strcmp(type, "VEC3") == 0 ){
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
        LwInputTexture* pInput = new LwInputTexture(loc, value, _textureUnit++);
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
    
    //texture
    m_puiTextureIDs = new GLuint[_pod.nNumMaterial];
    
	if(!m_puiTextureIDs)
	{
		lwerror("ERROR: Insufficient memory.");
        lwassert(0);
		return;
	}
    
	for(int i = 0; i < (int) _pod.nNumMaterial; ++i)
	{
		m_puiTextureIDs[i] = 0;
		SPODMaterial* pMaterial = &_pod.pMaterial[i];
        
		if(pMaterial->nIdxTexDiffuse != -1)
		{
			/*
             Using the tools function PVRTTextureLoadFromPVR load the textures required by the pod file.
             
             Note: This function only loads .pvr files. You can set the textures in 3D Studio Max to .pvr
             files using the PVRTexTool plug-in for max. Alternatively, the pod material properties can be
             modified in PVRShaman.
             */
            
			const char* sTextureName = _pod.pTexture[pMaterial->nIdxTexDiffuse].pszName;
            
			if(PVRTTextureLoadFromPVR(_f(sTextureName), &m_puiTextureIDs[i]) != PVR_SUCCESS)
			{
				lwerror("only pvr");
                lwassert(0);
				return;
			}
		}
	}

    _pEffects = lw::EffectsRes::create("texture.lwfx");
    _posLoc = _pEffects->getLocationFromSemantic(lw::EffectsRes::POSITION);
    _uvLoc = _pEffects->getLocationFromSemantic(lw::EffectsRes::UV0);
    _mvpLoc = _pEffects->getLocationFromSemantic(lw::EffectsRes::WORLDVIEWPROJ);
    _samplerLoc = _pEffects->getUniformLocation("u_texture");
}

LwModel::~LwModel(){
    if ( _vbos )
        glDeleteBuffers(_vbosNum, _vbos);
        delete [] _vbos;
    if ( _indexVbos )
        glDeleteBuffers(_vbosNum, _indexVbos);
        delete [] _indexVbos;
    glDeleteTextures(_pod.nNumMaterial, &m_puiTextureIDs[0]);
    _pEffects->release();
    
    std::vector<LwMesh*>::iterator it = _meshes.begin();
    std::vector<LwMesh*>::iterator itend = _meshes.end();
    for ( ; it != itend; ++it ){
        delete (*it);
    }
}

void LwModel::draw(){
    _pEffects->use();
    
    for (unsigned int i = 0; i < _pod.nNumMeshNode; ++i)
	{
		SPODNode& Node = _pod.pNode[i];
        
		// Get the node model matrix
		PVRTMat4 mWorld;
		mWorld = _pod.GetWorldMatrix(Node);
        
        PVRTMat4 mScale = PVRTMat4::Scale(.07f, .07f, .07f);
        mWorld = mScale * mWorld;
        PVRTMat4 mTrans = PVRTMat4::Translation(0.f, 2.f, 0.f);
        mWorld = mTrans * mWorld;
        
		// Pass the model-view-projection matrix (MVP) to the shader to transform the vertices
		PVRTMat4 mMVP = _viewProjMat * mWorld;
		glUniformMatrix4fv(_mvpLoc, 1, GL_FALSE, mMVP.f);
        
		// Load the correct texture using our texture lookup table
		GLuint uiTex = 0;
        
		if(Node.nIdxMaterial != -1)
			uiTex = m_puiTextureIDs[Node.nIdxMaterial];
        
		glBindTexture(GL_TEXTURE_2D, uiTex);
        
		/*
         Now that the model-view matrix is set and the materials ready,
         call another function to actually draw the mesh.
         */
		drawMesh(i);
	}
}

void LwModel::setViewProj(const cml::Matrix4& mat){
    memcpy(_viewProjMat.f, mat.data(), sizeof(float)*16);
}

void LwModel::drawMesh(int i32NodeIndex)
{
	int i32MeshIndex = _pod.pNode[i32NodeIndex].nIdx;
	SPODMesh* pMesh = &_pod.pMesh[i32MeshIndex];
    
	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, _vbos[i32MeshIndex]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexVbos[i32MeshIndex]);
    
	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(_posLoc);
	glEnableVertexAttribArray(_uvLoc);
    
	// Set the vertex attribute offsets
	glVertexAttribPointer(_posLoc, 3, GL_FLOAT, GL_FALSE, pMesh->sVertex.nStride, pMesh->sVertex.pData);
	//glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sNormals.nStride, pMesh->sNormals.pData);
	glVertexAttribPointer(_uvLoc, 2, GL_FLOAT, GL_FALSE, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);
    
	/*
     The geometry can be exported in 4 ways:
     - Indexed Triangle list
     - Non-Indexed Triangle list
     - Indexed Triangle strips
     - Non-Indexed Triangle strips
     */
	if(pMesh->nNumStrips == 0)
	{
		if(_indexVbos[i32MeshIndex])
		{
			// Indexed Triangle list
			glDrawElements(GL_TRIANGLES, pMesh->nNumFaces*3, GL_UNSIGNED_SHORT, 0);
		}
		else
		{
			// Non-Indexed Triangle list
			glDrawArrays(GL_TRIANGLES, 0, pMesh->nNumFaces*3);
		}
	}
	else
	{
		int offset = 0;
        
		for(int i = 0; i < (int)pMesh->nNumStrips; ++i)
		{
			if(_indexVbos[i32MeshIndex])
			{
				// Indexed Triangle strips
				glDrawElements(GL_TRIANGLE_STRIP, pMesh->pnStripLength[i]+2, GL_UNSIGNED_SHORT, &((GLshort*)0)[offset]);
			}
			else
			{
				// Non-Indexed Triangle strips
				glDrawArrays(GL_TRIANGLE_STRIP, offset, pMesh->pnStripLength[i]+2);
			}
			offset += pMesh->pnStripLength[i]+2;
		}
	}
    
	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(_posLoc);
	//glDisableVertexAttribArray(NORMAL_ARRAY);
	glDisableVertexAttribArray(_uvLoc);
    
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

