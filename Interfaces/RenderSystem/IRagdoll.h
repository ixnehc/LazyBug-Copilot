
#pragma once

#include "IResource.h"


class IRagdoll:public IResource
{
public:
	virtual BYTE *GetData(DWORD &szData)=0;
};

class IRagdollMgr:public IResourceMgr
{

};
