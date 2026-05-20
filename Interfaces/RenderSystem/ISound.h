
#pragma once

#include "IResource.h"

class ISoundPlay
{
public:
	INTERFACE_REFCOUNT;
};


class ISound:public IResource
{
public:
	virtual ISoundPlay *Play2D(BOOL bLoop=FALSE)=0;
	virtual ISoundPlay *Play3D(i_math::vector3df &pos,BOOL bLoop=FALSE)=0;
};

class ISoundMgr:public IResourceMgr
{

};
