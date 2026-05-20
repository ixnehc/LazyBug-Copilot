#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_DemonBlood 64

struct EoParamDemonBlood:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamDemonBlood);

	BEGIN_GOBJ_PURE(EoParamDemonBlood,1);

		GELEM_VAR_INIT(float,speed,5.0f);
			GELEM_EDITVAR("飞行速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"飞行速度");

	END_GOBJ();

	float speed;
};

struct EoDemonBloodArg
{
	EoDemonBloodArg()
	{
		idSrc=LevelObjID_Invalid;
		strInitial=0.0f;
	}
	LevelObjID idSrc;

	LevelPos dirInitial;//初始方向
	float strInitial;//初始速度
};


class EoDemonBlood:public CLoEffectObj
{
public:
	EoDemonBlood()
	{
	}
	DEFINE_LEVELOBJ_CLASS(EoDemonBlood,CLASSUID_DemonBlood);

	void Init(EoDemonBloodArg &arg);

	virtual const char *GetShowName()	{		return "魔血";	}

//	void Confirm();

protected:

	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	void _OnUpdate();

	EoDemonBloodArg _arg;


};
