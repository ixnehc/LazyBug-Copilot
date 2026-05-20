
#include "stdh.h"


#include "Reactor_DamageImmune.h"

#include "Random/Random.h"


//////////////////////////////////////////////////////////////////////////
//Reactor_DamageImmune
BIND_REACTORPARAM(Reactor_DamageImmune,ReactorParam_DamageImmune);

void Reactor_DamageImmune::_OnCreate()
{
}

void Reactor_DamageImmune::HandleEvent(LevelEvent &e0)
{
	ReactorParam_DamageImmune *param=(ReactorParam_DamageImmune *)_param;
	if (e0.GetType()==LET_PreDamage)
	{
		LePreDamage *e=(LePreDamage *)&e0;
		if (CSysRandom::Roll(param->possibility))
		{
			BOOL bConditionMet=FALSE;
			switch(param->condition)
			{
				case ReactorParam_DamageImmune::Condition_Always:
				{
					bConditionMet=TRUE;
					break;
				}
				case ReactorParam_DamageImmune::Condition_NotCastingSkillAndDmgFromThreat:
				{
					CLevelObj *owner=_GetOwner();
					if (owner)
					{
						extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
						if (!LevelUtil_GetCastingSkill(owner))
						{//没有释放技能
							if (e->osb)
							{
								extern CLevelObj *LevelUtil_GetThreat(CLevelObj *lo);
								CLevelObj *loThreat=LevelUtil_GetThreat(owner);
								if (loThreat)
								{
									if (e->osb->GetRootOwnerID()==loThreat->GetID())
										bConditionMet=TRUE;
								}
							}
						}
					}
					break;
				}
			}

			if (bConditionMet)
			{
				e->scaleDmg-=param->scaleReduced;
				if (e->scaleDmg<0.0f)
					e->scaleDmg=0.0f;
			}
		}
	}
}
