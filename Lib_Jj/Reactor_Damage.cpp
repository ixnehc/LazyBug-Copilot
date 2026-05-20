
#include "stdh.h"


#include "Reactor_Damage.h"


//////////////////////////////////////////////////////////////////////////
//Reactor_Damage
BIND_REACTORPARAM(Reactor_Damage,ReactorParam_Damage);

void Reactor_Damage::_OnCreate()
{
}

void Reactor_Damage::HandleEvent(LevelEvent &e0)
{
	ReactorParam_Damage *param=(ReactorParam_Damage *)_param;
	if (e0.GetType()==LET_Damage)
	{
		LeDamage *e=(LeDamage *)&e0;
		if ((e->nDmg>=param->nMinDmg)&&(e->nDmg<=param->nMaxDmg))
		{
			DealArg arg;
			arg.link=e->link;
			param->dealToMe->Make(_GetOSB(),_GetOwner(),arg,NULL);
		}
	}
}
