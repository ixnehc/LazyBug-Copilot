#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_ObjDeal 67

struct EoParamObjDeal:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamObjDeal);

	BEGIN_GOBJ_PURE(EoParamObjDeal,1);

		GELEM_VAR_INIT(int,tpTarget,0);
			GELEM_EDITVAR("Obj类型",GVT_U,GSem(GSem_Interger,"Owner:0,技能Target:1"),"Obj类型");

	END_GOBJ();

	int tpTarget;


};



class EoObjDeal:public CLoEffectObj
{
public:
	EoObjDeal()
	{
		_idObj=LevelObjID_Invalid;
		_tCasting=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoObjDeal,CLASSUID_ObjDeal);

	virtual const char *GetShowName()	{		return "ObjDeal";	}

protected:
	virtual void _OnPostCreate();
	virtual void _OnUpdate();
	virtual void _OnDetroy();

	LevelObjID _idObj;
	AnimTick _tCasting;

};
