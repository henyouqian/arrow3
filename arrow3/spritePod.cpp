#include "stdafx.h"
#include "spritePod.h"
#include "PVRT/PVRTModelPOD.h"

SpritePod::SpritePod(const char *file, const char *nodeName)
:_posX(0.f), _posY(0.f), _rotate(0.f), _scaleX(1.f), _scaleY(1.f), _needUpdate(true){
    lwassert(file && nodeName);
    CPVRTModelPOD pod;
    if(pod.ReadFromFile(_f(file)) != PVR_SUCCESS){
        lwerror("Failed to load pod file. file=" << file);
        return;
    }
    
    //find node
    SPODNode* pNode = NULL;
    for ( int i = 0; i < (int)pod.nNumNode; ++i ){
        if ( strcmp(pod.pNode[i].pszName, nodeName) == 0 ){
            pNode = pod.pNode + i;
        }
    }
    if ( pNode == NULL ){
        lwerror("node is not found: nodeName=" << nodeName);
        return;
    }
    if ( pNode->nIdx >= (int)pod.nNumMesh ){
        lwerror("invalid mesh index");
        return;
    }
    SPODMesh* pMesh = pod.pMesh+pNode->nIdx;
    
    char* bp = (char*)pMesh->sVertex.pData;
    unsigned int stride = pMesh->sVertex.nStride;
    unsigned short* pIndex = (unsigned short*)pMesh->sFaces.pData;
    char* bpUV = (char*)pMesh->psUVW[0].pData;
    unsigned int strideUV = pMesh->psUVW[0].nStride;
    float* pv = NULL;
    
    lw::SpriteVertex vtx;
    for ( unsigned int j = 0; j < pMesh->nNumFaces; ++j ){
        for ( int ii = 0; ii < 3; ++ii ){
            pv = (float*)(bp + (*pIndex)*stride);
            vtx.x = *pv;
            vtx.y = -*(pv+1);
            vtx.z = 0;
            pv = (float*)(bpUV + (*pIndex)*strideUV);
            vtx.u = *pv;
            vtx.v = 1-*(pv+1);
            _vertices.push_back(vtx);
            ++pIndex;
        }
    }
    
    SPODMaterial* pMaterial = &pod.pMaterial[pNode->nIdxMaterial];
    int textureIndex = pMaterial->nIdxTexDiffuse;
    if ( textureIndex == -1 ){
        lwerror("no texture.");
        _pTexRes = NULL;
        return;
    }else{
        const char* textureName = pod.pTexture[textureIndex].pszName;
        if ( textureName == NULL ){
            lwerror("textureName == NULL");
            return;
        }
        _pTexRes = lw::TextureRes::create(textureName);
        if ( _pTexRes == NULL ){
            lwerror("lw::TextureRes::create failed");
            return;
        }
    }
    
    _color = lw::COLOR_WHITE;
    _blendMode = lw::BLEND_NORMAL;
}

SpritePod::~SpritePod(){
    if ( _pTexRes ){
        _pTexRes->release();
    }
}

void SpritePod::draw(){
    if ( _vertices.empty() ){
        return;
    }
    if ( _needUpdate ){
        update();
    }
    lw::spriteCollectVetices(&(_transVertices.front()), _transVertices.size(), _color, _blendMode, _pTexRes->getGlId());
}


void SpritePod::setColor(const lw::Color &color){
    _color = color;
}

void SpritePod::setBlendMode(lw::BlendMode blendMode){
    _blendMode = blendMode;
}

void SpritePod::setPos(float x, float y){
    if ( _posX != x || _posY != y ){
        _needUpdate = true;
        _posX = x;
        _posY = y;
    }
}

void SpritePod::setRotate(float r){
    if ( _rotate != r ){
        _needUpdate = true;
        _rotate = r;
    }
}

void SpritePod::setScale(float x, float y){
    if ( _scaleX != x || _scaleY != y ){
        _needUpdate = true;
        _scaleX = x;
        _scaleY = y;
    }
}

void SpritePod::update(){
    _needUpdate = false;
    
    cml::Matrix4 m, m1;
    cml::matrix_translation(m, _posX, _posY, 0.f);
    if ( _rotate != 0.f ){
        cml::matrix_rotation_world_z(m1, _rotate);
        m *= m1;
    }
    if ( _scaleX != 1.f || _scaleY != 1.f ){
        cml::matrix_scale(m1, _scaleX, _scaleY, 1.f);
        m *= m1;
    }
    
    _transVertices.clear();
    std::vector<lw::SpriteVertex>::iterator it = _vertices.begin();
    std::vector<lw::SpriteVertex>::iterator itend = _vertices.end();
    lw::SpriteVertex vtx;
    for ( ; it != itend; ++it ){
        cml::Vector3 v(it->x, it->y, it->z);
        v = cml::transform_point(m, v);
        vtx = *it;
        vtx.x = v[0];
        vtx.y = -v[1];
        _transVertices.push_back(vtx);
    }
}




