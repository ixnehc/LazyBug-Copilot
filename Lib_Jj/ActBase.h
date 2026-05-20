#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "records/recordsdefine.h"

#include "LevelDefines.h"

#define DEFINE_ACT_CLASS(clss)									\
DEFINE_CLASS(clss);														\
virtual CClass *GetParamClass();

#define BIND_ACT_PARAM(clss,clssParam) CClass *clss::GetParamClass(){	return Class_Ptr2(clssParam);}

class CLevelObj;



struct ActParam
{
	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()	{		return NULL;	}
	virtual void CopyFrom(ActParam *paramSrc)
	{
		GObjBase *p1,*p2;
		p1=GetGObj();
		p2=paramSrc->GetGObj();
		if (p1&&p2)
			p1->Copy(p2);
	}

};


struct ActParam;
class ActBase
{
public:
	ActBase()
	{
		_owner=NULL;
		_param=NULL;
	}
	~ActBase()
	{
		Safe_Class_Delete(_param);
	}
	virtual CClass *GetClass()=0;
	virtual CClass *GetParamClass()=0;
	void Set(CLevelObj *owner,ActParam *param);
	virtual void Update(AnimTick t)=0;

	virtual void Finish()=0;


protected:

	CLevelObj *_owner;
	ActParam *_param;

};


