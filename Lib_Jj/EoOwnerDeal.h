#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_OwnerDeal 47

struct EoParamOwnerDeal:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamOwnerDeal);

	EoParamOwnerDeal()
	{
	}

	~EoParamOwnerDeal()
	{
	}


	BEGIN_GOBJ(EoParamOwnerDeal,1);


	END_GOBJ();

	BOOL bDefered;

};



class EoOwnerDeal:public CLoEffectObj
{
public:
	EoOwnerDeal()
	{
		_idOwner=LevelObjID_Invalid;
		_tCasting=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoOwnerDeal,CLASSUID_OwnerDeal);

	virtual const char *GetShowName()	{		return "OwnerDeal";	}

protected:
	virtual void _OnPostCreate();
	virtual void _OnUpdate();
	virtual void _OnDetroy();

	LevelObjID _idOwner;
	AnimTick _tCasting;

};
