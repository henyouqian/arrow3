#include "stdafx.h"
#include "podModel.h"
#include "PVRT/OGLES2/PVRTTextureAPI.h"

PodModel::PodModel(const char *podFile){
    _vbos = _indexVbos = NULL;
    lwassert(podFile);
    
    if(_pod.ReadFromFile(_f(podFile)) != PVR_SUCCESS){
        lwerror("Failed to load pod file. file=" << podFile);
        return;
    }
    
    if(!_pod.pMesh[0].pInterleaved){
		lwerror("requires the pod data to be interleaved");
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
    
    //texture
    m_puiTextureIDs = new GLuint[_pod.nNumMaterial];
    
	if(!m_puiTextureIDs)
	{
		lwerror("ERROR: Insufficient memory.");
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
				return;
			}
		}
	}

    _pEffects = lw::EffectsRes::create("texture.lwfx");
    _posLoc = _pEffects->getLocationFromSemantic(lw::EffectsRes::POSITION);
    _uvLoc = _pEffects->getLocationFromSemantic(lw::EffectsRes::UV0);
    _mvpLoc = _pEffects->getLocationFromSemantic(lw::EffectsRes::WORLDVIEWPROJ);
    _samplerLoc = _pEffects->getUniformLocation("u_texture");
    _colorLoc = _pEffects->getUniformLocation("u_color");
}

PodModel::~PodModel(){
    if ( _vbos )
        glDeleteBuffers(_vbosNum, _vbos);
        delete [] _vbos;
    if ( _indexVbos )
        glDeleteBuffers(_vbosNum, _indexVbos);
        delete [] _indexVbos;
    glDeleteTextures(_pod.nNumMaterial, &m_puiTextureIDs[0]);
    _pEffects->release();
}

void PodModel::draw(){
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
        
        glUniform4f(_colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        
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

void PodModel::setViewProj(const cml::Matrix4& mat){
    memcpy(_viewProjMat.f, mat.data(), sizeof(float)*16);
}

void PodModel::drawMesh(int i32NodeIndex)
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
