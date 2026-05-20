#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#include "LevelOps.h"

#define CLASSUID_Trap 27

struct EoParamTrap:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamTrap);

	struct LevelUp
	{
		BEGIN_GOBJ_PURE(LevelUp,1);

			GELEM_VAR_INIT(AnimTick,t,ANIMTICK_FROM_SECOND(0.0f));
				GELEM_EDITVAR("开始时间",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"开始时间");

			GELEM_OBJVECTOR(DealEntry,deals); 
				GELEM_EDITOBJ("结算列表","多个结算");

		END_GOBJ();

		AnimTick t;
		std::vector<DealEntry> deals;

	};

	BEGIN_GOBJ_PURE(EoParamTrap,1);

		GELEM_VAR_INIT(AnimTick,durLife,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("生命时间",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"Trap持续多久,0表示永远");
		GELEM_VAR_INIT(AnimTick,durSafe,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("安全时间",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"一开始多长时间内无法触发");

		GELEM_VAR_INIT(float,radius,2.0f);
			GELEM_EDITVAR("作用范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"作用范围");

		GELEM_VAR_INIT(float,radiusDetect,1.0f);
			GELEM_EDITVAR("感应范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"感应范围");

		GELEM_VAR_INIT(AnimTick,delay,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("延迟作用时间",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"延迟多久产生作用");

		GELEM_OBJVECTOR(LevelUp,levelups);
			GELEM_EDITOBJ("LevelUps","LevelUps");

		GELEM_VAR_INIT(BOOL,bEnvBound,FALSE);
			GELEM_EDITVAR("战斗环境绑定",GVT_S,GSem_Boolean,"战斗环境结束后自动删除");
	END_GOBJ();

	int GetLevelUp(AnimTick t)
	{
		int iLevelUp=-1;
		for (int i=0;i<levelups.size();i++)
		{
			if (levelups[i].t<t)
			{
				if (iLevelUp<0)
					iLevelUp=i;
				else
				{
					if (levelups[i].t>levelups[iLevelUp].t)
						iLevelUp=i;
				}
			}
		}
		return iLevelUp;
	}

	AnimTick durLife;
	AnimTick durSafe;
	float radiusDetect;
	float radius;
	AnimTick delay;
	std::vector<LevelUp> levelups;
	BOOL bEnvBound;
};



class EoTrap:public CLoEffectObj
{
public:
	EoTrap()
	{
		_tTriggered=0;
		_bTriggered=FALSE;
		_iLevelUp=-1;

		_bBurst=FALSE;
	}
	DEFINE_LEVELOBJ_CLASS(EoTrap,CLASSUID_Trap);

	virtual const char *GetShowName()	{		return "陷阱";	}

protected:

	virtual void _OnDetroy();
	virtual BOOL _NeedOps()	{		return TRUE;	}

	void _OnUpdate();

	BOOL _bTriggered;//有没有触发
	LevelTick _tTriggered;

	BOOL _bBurst;

	int _iLevelUp;


};
