
#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GStub.h"
#include "gds/GProp.h"

#include "stubparams.h"
#include "param_ael.h"
#include "param_mano.h"

#include "../ILuaMachine.h"


class IAnimNode;

struct Prop_AnimNode:public GProperty
{
public:
	DEFINE_CLASS(Prop_AnimNode);


	virtual GProperty *Clone();
	virtual void DeleteThis();
	virtual GVarType GetGVT()	{		return (GVarType)GVTEx_AnimNode;	}


	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(Prop_AnimNode,1);
		GELEM_VAR_INIT(IAnimNode*,an,NULL);
	END_GOBJ();    

	IAnimNode*an;
};

struct Prop_AvatarSyncData:public GProperty
{
public:
	DEFINE_CLASS(Prop_AvatarSyncData);

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(Prop_AvatarSyncData,1);
		GELEM_VARVECTOR(BYTE,buf);
	END_GOBJ();    
	
	std::vector<BYTE> buf;
};

class CAvtrStates;
struct Prop_AvatarStates:public GProperty
{
public:
	DEFINE_CLASS(Prop_AvatarStates);

	virtual GProperty *Clone();
	virtual void DeleteThis();
	virtual GVarType GetGVT()	{		return (GVarType)GVTEx_AvtrStates;	}


	BEGIN_GOBJ_PURE(Prop_AvatarStates,1);
		GELEM_VAR_INIT(CAvtrStates *,v,NULL);
	END_GOBJ();    

	CAvtrStates *v;
};


inline BOOL Prop_ParseLink(GProperty *prop,IAnimNode *&an)
{
	an=NULL;
	if (!prop)
		return FALSE;

	if (prop->GetGVT()==GVTEx_AEL)
	{
		prop=(((PropAEL *)prop)->v.Call("GetLink",&Prop_Void()));
		if (!prop)
			return FALSE;
	}

	switch(prop->GetGVT())
	{
	case GVTEx_AnimNode:
		{
			an=((Prop_AnimNode*)prop)->an;
			return an!=NULL;
		}
	}

	if (TRUE)
	{
		Prop_AnimNode propAnimNode;
		Prop_AnimNode *prop2=(Prop_AnimNode *)prop->To(&propAnimNode);
		if (prop2)
		{
			an=prop2->an;
			return an!=NULL;
		}
	}


	return FALSE;
}

//返回能否在params的指定位置(idx)找到一个Link
inline BOOL StbParams_ParseLink(StbParams *params,int idx,IAnimNode *&an)
{
	return Prop_ParseLink(params->GetObj(idx),an);
}
