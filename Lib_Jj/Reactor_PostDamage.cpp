
#include "stdh.h"


#include "Reactor_PostDamage.h"


//////////////////////////////////////////////////////////////////////////
//Reactor_PostDamage
BIND_REACTORPARAM(Reactor_PostDamage,ReactorParam_PostDamage);

void Reactor_PostDamage::_OnCreate()
{
}
 
void Reactor_PostDamage::HandleEvent(LevelEvent &e0)
{
	ReactorParam_PostDamage *param=(ReactorParam_PostDamage *)_param;
	if (e0.GetType()==LET_PostDamage)
	{
		LePostDamage *e=(LePostDamage *)&e0;

		if (e->loTarget==_GetOwner())
		{
			DealArg arg;
			arg.dir.setXZ(e->strike->GetDir());
			arg.link=e->link;
			if (e->osbSrc)
				param->dealToMe->Make(*e->osbSrc,_GetOwner(),arg,NULL);
			else
				param->dealToMe->Make(_GetOSB(),_GetOwner(),arg,NULL);

			e->bAbandon=TRUE;
		}
	}
}
