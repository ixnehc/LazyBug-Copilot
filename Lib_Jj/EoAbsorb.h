#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Absorb 23

struct EoParamAbsorb:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamAbsorb);

	BEGIN_GOBJ_PURE(EoParamAbsorb,1);

		GELEM_VAR_INIT(float,speed,5.0f);
			GELEM_EDITVAR("飞行速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"飞行速度");

	END_GOBJ();

	float speed;
};

struct EoAbsorbArg
{
	EoAbsorbArg()
	{
		idSrc=LevelObjID_Invalid;
		idTarget=LevelObjID_Invalid;
		strInitial=0.0f;
	}
	LevelObjID idSrc;
	LevelObjID idTarget;

	LevelPos dirInitial;//初始方向
	float strInitial;//初始速度
};


class EoAbsorb:public CLoEffectObj
{
public:
	EoAbsorb()
	{
	}
	DEFINE_LEVELOBJ_CLASS(EoAbsorb,CLASSUID_Absorb);

	void Init(EoAbsorbArg &arg);

	virtual const char *GetShowName()	{		return "吸取";	}

	void Confirm();

protected:

	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	void _OnUpdate();

	EoAbsorbArg _arg;


};
