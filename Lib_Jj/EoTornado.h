#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Tornado 40

struct EoParamTornado:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamTornado);

	BEGIN_GOBJ_PURE(EoParamTornado,1);

		GELEM_VAR_INIT(float,radius,0.2f);
			GELEM_EDITVAR("半径",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"半径");
		GELEM_VAR_INIT(float,speedMin,2.0f);
			GELEM_EDITVAR("最小速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"速度");
		GELEM_VAR_INIT(float,speedMax,4.0f);
			GELEM_EDITVAR("最大速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"速度");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续时间");
		GELEM_VAR_INIT(AnimTick,cycleDmg,ANIMTICK_FROM_SECOND(0.2f));
			GELEM_EDITVAR("伤害间隔",GVT_U,GSem(GSem_AnimTick,"0,100,0.02"),"伤害间隔");
		GELEM_VAR_INIT(float,radiusDmg,1.0f);
			GELEM_EDITVAR("伤害半径",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"伤害半径");
	END_GOBJ();

	float radius;
	float speedMin;
	float speedMax;
	float deflectMin;//最小偏转速度
	float deflectMax;//最大偏转速度
	AnimTick dur;
	AnimTick accDeflect;//偏转速度的加速度

	AnimTick cycleDmg;
	float radiusDmg;
};

class EoTornado:public CLoEffectObj
{
public:
	EoTornado()
	{
		_nDamaged=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoTornado,CLASSUID_Tornado);

	struct State
	{
		State()
		{
			t=0;
			face=0.0f;
			deflect=0.0f;
			deflectMax=0.0f;
			speed=0.0f;
			signDeflect=1.0f;
		}
		AnimTick t;
		LevelPos pos;
		float face;
		float speed;
		float deflect;
		float deflectMax;
		float signDeflect;
	};

	virtual const char *GetShowName()	{		return "龙卷风";	}
	virtual LevelPos GetFramePos()	{		return _state.pos;	}

protected:

	virtual void _OnPostCreate();


	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	void _OnWriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	void _OnUpdate();
	virtual BOOL _NeedOps()	{		return TRUE;	}

	void _WriteState(CBitPacket *bp);

	State _state;

	int _nDamaged;


};
