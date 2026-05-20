#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "LevelObliterateArg.h"


#include "LoEffectObj.h"

#define CLASSUID_Obliterater 39

struct EoParamObliterater:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamObliterater);

	BEGIN_GOBJ_PURE(EoParamObliterater,1);

		GELEM_VAR_INIT(float,radius,2.0f);
			GELEM_EDITVAR("作用范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"作用范围");

		GELEM_VAR_INIT(float,strKill,0.0f);
			GELEM_EDITVAR("击飞力量",GVT_F,GSem(GSem_Float,"0,100,0.1"),"杀死敌人后用多大力量击飞敌人");

	END_GOBJ();

	float radius;
	float strKill;//杀死敌人后,用多大的力量击飞敌人
};



class EoObliterater:public CLoEffectObj
{
public:
	EoObliterater()
	{
		_bBurst=FALSE;
	}
	DEFINE_LEVELOBJ_CLASS(EoObliterater,CLASSUID_Obliterater);

	virtual const char *GetShowName()	{		return "单位爆裂";	}

protected:
	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;

	virtual void _OnPostCreate() override;
	virtual void _OnUpdate() override;

	virtual void SetObliterateArg(LevelObliterateArg &arg) override;

	LevelObliterateArg _arg;

	BOOL _bBurst;



};
