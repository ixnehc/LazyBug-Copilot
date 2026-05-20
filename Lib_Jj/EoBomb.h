#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Bomb 21

struct EoParamBomb:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamBomb);

	BEGIN_GOBJ_PURE(EoParamBomb,1);

		GELEM_VAR_INIT(BOOL,bHostBomb,FALSE);
			GELEM_EDITVAR("单位身上的Bomb",GVT_S,GSem_Boolean,"单位身上的Bomb");
		GELEM_VAR_INIT(float,radius,2.0f);
			GELEM_EDITVAR("作用范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"作用范围");

		GELEM_VAR_INIT(AnimTick,delay,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("延迟爆炸时间",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"延迟多久爆炸");

		GELEM_VAR_INIT(BOOL,bIgnoreHost,FALSE);
			GELEM_EDITVAR("伤害时忽略依附的单位",GVT_S,GSem_Boolean,"伤害时忽略依附的单位");

	END_GOBJ();

	float radius;
	AnimTick delay;
	float dmg;//伤害
	BOOL bHostBomb;
	BOOL bIgnoreHost;
};



class EoBomb:public CLoEffectObj
{
public:
	EoBomb()
	{
		_bBurst=FALSE;
	}
	DEFINE_LEVELOBJ_CLASS(EoBomb,CLASSUID_Bomb);

	virtual const char *GetShowName()	{		return "炸弹";	}

protected:
	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;

	void _OnUpdate();

	BOOL _bBurst;//初始的Damage有没有结算

};
