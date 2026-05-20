
#include "stdh.h"


#include "Reactor_Stun.h"


//////////////////////////////////////////////////////////////////////////
//Reactor_Stun
BIND_REACTORPARAM(Reactor_Stun,ReactorParam_Stun);

void Reactor_Stun::_OnCreate()
{
}

void Reactor_Stun::HandleEvent(LevelEvent &e0)
{
	ReactorParam_Stun *param=(ReactorParam_Stun *)_param;
	if (e0.GetType()==LET_Stun)
	{
		if (!param->bDefered)
		{
			LeDamage *e=(LeDamage *)&e0;
			DealArg arg;
			arg.link=e->link;
			param->dealToMe->Make(_GetOSB(),_GetOwner(),arg,NULL);
		}
		else
			_bTriggered=TRUE;
	}
}

void Reactor_Stun::Update(AnimTick t)
{
	if (_bTriggered)
	{
		ReactorParam_Stun *param=(ReactorParam_Stun *)_param;
		DealArg arg;
		param->dealToMe->Make(_GetOSB(),_GetOwner(),arg,NULL);
		_bTriggered=FALSE;
	}

}
