#pragma once

#include "LevelBuff.h"

#include "LevelDeal.h"

struct CLevelDeal;
struct BuffParam_Deal:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Deal);

	BEGIN_GOBJ_PURE(BuffParam_Deal,1);

		GELEM_VAR_INIT(int,modeTarget,2);
			GELEM_EDITVAR("作用方式",GVT_U,GSem(GSem_Interger,
				"作用于对象"	"|对象位置类型,"
				"作用于对象位置,"
				"既作用于对象也作用于对象位置"
				),"作用方式");

		GELEM_VAR_INIT(int,tpTargetPos,0);
			GELEM_EDITVAR("对象位置类型",GVT_U,GSem(GSem_Interger,"脚下,瞄准点"),"对象位置类型");

		GELEM_VAR_INIT(BOOL,bContinuous,FALSE);
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,
				"持续Deal:1"		"|延迟,"
				"只Deal一次:0"	"|每秒Deal几次"
				),"工作模式");

		GELEM_VAR_INIT(AnimTick,delay,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("延迟",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"延迟执行Action");
		GELEM_VAR_INIT(float,dps,5.0f);
			GELEM_EDITVAR("每秒Deal几次",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"每秒Deal几次");
		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_Dmg, "Action(Obsolete)", "Action" );
			GELEMS_LEVELDEAL_CANDIDATES();

	END_GOBJ();

	int modeTarget;
	int tpTargetPos;

	BOOL bContinuous;

	CLevelDeal *deal;
	float dps;
	AnimTick delay;

};


struct BuffArg_Deal:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Deal)
};


class Buff_Deal:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Deal,60)

	Buff_Deal()
	{
		_bDealed=FALSE;
		_nDealed=0;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端


	virtual void _OnCreate(LevelBuffArg *param)
	{
	}


protected:

	virtual void _OnUpdate(AnimTick dt);
	BOOL _bDealed;
	int _nDealed;



};

