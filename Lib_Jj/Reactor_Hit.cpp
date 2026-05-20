
#include "stdh.h"


#include "Reactor_Hit.h"


//////////////////////////////////////////////////////////////////////////
//Reactor_Hit
BIND_REACTORPARAM(Reactor_Hit,ReactorParam_Hit);

void Reactor_Hit::_OnCreate()
{
}

void Reactor_Hit::HandleEvent(LevelEvent &e0)
{
	ReactorParam_Hit *param=(ReactorParam_Hit *)_param;
	if (e0.GetType()==LET_Hit)
	{
		if (!param->bDefered)
		{
			LeDamage *e=(LeDamage *)&e0;
			DealArg arg;
			arg.link=e->link;
			param->dealToMe->Make(_GetOSB(),_GetOwner(),arg,NULL);
// 			MakeDeals(param->dealsToMe,_GetOSB(),_GetOwner(),arg,NULL);
		}
		else
			_bTriggered=TRUE;
	}
}

void Reactor_Hit::Update(AnimTick t)
{
	if (_bTriggered)
	{
		ReactorParam_Hit *param=(ReactorParam_Hit *)_param;
		DealArg arg;
		param->dealToMe->Make(_GetOSB(),_GetOwner(),arg,NULL);
// 		MakeDeals(param->dealsToMe,_GetOSB(),_GetOwner(),arg,NULL);
		_bTriggered=FALSE;
	}

}
