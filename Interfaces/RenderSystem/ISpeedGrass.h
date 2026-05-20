
#pragma once

#include "IResource.h"

class ITexture;

class ISpg:public IResource
{
public:
	virtual ITexture * GetTex_D()=0;
	virtual ITexture * GetTex_N()=0;
	virtual ITexture * GetTex_S()=0;
	virtual i_math::aabbox3df GetAabb() = 0;
	virtual i_math::spheref GetDefaultSphere() const = 0;

	// spg height
	virtual float GetHeight() = 0;
		
	// index buffer count
	virtual const WORD * GetIndexData(DWORD & c) = 0; 
	
	virtual DWORD GetNumberOfVertexs() = 0;
	virtual const i_math::vector4df & GetVertex(int idx) = 0;
	virtual const i_math::vector3df & GetNormal(int idx) = 0;
	virtual const i_math::vector2df & GetTexVertex(int idx) = 0;
};

class ISpgMgr:public IResourceMgr
{

};

struct SpgData;
class IDynSpgMgr:public IResourceMgr
{
public:
	virtual ISpg *Create(SpgData *data,const char * pathRes)=0;
};
