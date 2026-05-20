
#pragma once

#include "IResource.h"


typedef WORD MteID;
#define MteID_Invalid (0)

struct MtrlExtData;
class IMtrlExt:public IResource
{
public:
	virtual MteID GetID()=0;
	virtual MtrlExtData *GetData()=0;
};

class IMtrlExtMgr:public IResourceMgr
{
	virtual IMtrlExt *Create(MtrlExtData *data,const char *pathOverride)=0;
};

