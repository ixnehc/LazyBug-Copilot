
#pragma once

#include "IResource.h"


class IMopp:public IResource
{
public:
	virtual i_math::aabbox3df &GetAabb()=0;
	virtual i_math::vector3df *GetVertices(DWORD &count)=0;
	virtual WORD *GetIndices(DWORD &count)=0;
	virtual BYTE GetDataType() = 0;//0: hk, 1: px
	virtual BYTE *GetData(DWORD &count)=0;
	virtual DWORD GetNumberOfFaces() = 0;
	virtual BOOL GetFace(int idx,i_math::triangle3df & tri) = 0;
	virtual BOOL HitTest(const i_math::line3df & rayHit,DWORD &iFace,float &dist) = 0;
};

class IMoppMgr:public IResourceMgr
{

};
