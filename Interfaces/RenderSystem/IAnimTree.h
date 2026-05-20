
#pragma once

#include "IResource.h"

struct AnimTreeData;
class IAnimTree:public IResource
{
public:
	virtual AnimTreeData *GetData()=0;
};

class IAnimTreeMgr:public IResourceMgr
{

};

struct AnimTreeData;
class IDynAnimTreeMgr:public IResourceMgr
{
public:
	virtual IAnimTree *Create(AnimTreeData *data)=0;
};

