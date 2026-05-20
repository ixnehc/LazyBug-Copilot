
#include "stdh.h"


#include "Reactor_PreKill.h"


//////////////////////////////////////////////////////////////////////////
//Reactor_PreKill
BIND_REACTORPARAM(Reactor_PreKill,ReactorParam_PreKill);

void Reactor_PreKill::_OnCreate()
{
}

void Reactor_PreKill::HandleEvent(LevelEvent &e0)
{
	ReactorParam_PreKill *param=(ReactorParam_PreKill *)_param;
	if (e0.GetType()==LET_PreKill)
	{
		LePreKill *e=(LePreKill *)&e0;

		DealArg arg;
		arg.dir.setXZ(e->strike->GetDir());
		arg.link=e->link;
		param->dealToMe->Make(_GetOSB(),_GetOwner(),arg,NULL);

		e->bAbandon=param->bAbandon;
	}
}
