#pragma once

#include "LevelBuff.h"

#include "LevelDeal.h"

struct CLevelDeal;
struct BuffParam_Shoot:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Shoot);

	BEGIN_GOBJ_PURE(BuffParam_Shoot,1);

		GELEM_VAR_INIT(AnimTick,delay,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("延迟",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"延迟执行Action");
		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_CreateEo, "Action", "Action" );
			GELEM_DYNOBJPTR_CLASS_DEAL( "04.创建Eo", Deal_CreateEo);

	END_GOBJ();

	CLevelDeal *deal;
	AnimTick delay;

};


struct BuffArg_Shoot:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Shoot)
};


class Buff_Shoot:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Shoot,25)

	Buff_Shoot()
	{
		_bShooted=FALSE;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端


	virtual void _OnCreate(LevelBuffArg *param)
	{
	}


protected:

	virtual void _OnUpdate(AnimTick dt);
	BOOL _bShooted;



};

