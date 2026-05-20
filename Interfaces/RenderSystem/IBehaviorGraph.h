
#pragma once

#include "IResource.h"
#include "strlib/strlibdefines.h"

struct BehaviorGraphData;
class IBehaviorGraph:public IResource
{
public:
	virtual BehaviorGraphData *GetData()=0;
};

struct LinkPadClasses;
class IBehaviorGraphMgr:public IResourceMgr
{
public:
	virtual void SetClasses(LinkPadClasses *clsses)=0;
	virtual LinkPadClasses *GetClasses()=0;
	virtual IResource *ObtainRes(StringID nmBG)=0;

};

struct BehaviorGraphData;
class IDynBehaviorGraphMgr:public IResourceMgr
{
public:
	virtual IBehaviorGraph*Create(BehaviorGraphData *data)=0;
};

