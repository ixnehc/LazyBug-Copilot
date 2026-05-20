/********************************************************************
	created:	2016/06/24 
	author:		cxi
	
	purpose:	BurningµÄBuff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"


#include "Buff_Burning.h"



//////////////////////////////////////////////////////////////////////////
//CBuff_Burning
BIND_BUFFPARAM(Buff_Burning,BuffParam_Burning,BuffArg_Burning);

LevelBuffMask Buff_Burning::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Burning)->GetUID());

	return mask;
}

void Buff_Burning::_OnUpdate(AnimTick dt)
{
	BuffParam_Burning *param=(BuffParam_Burning*)_param;
	int nToDeal=_tAge/param->gap;

	DealArg arg;
	CLevelObj *lo=_GetOwner();
	while (nToDeal>_nDeal)
	{
		_MakeDeals(lo,arg);
		_nDeal++;
	}
}

float Buff_Burning::_CalcDPS(LevelRecordBuff *rec)
{
	BuffParam_Burning *param=rec->GetParam<BuffParam_Burning>();
	if (!param)
		return 0.0f;
	float dmg=0.0f;

	for (int i=0;i<rec->deals.size();i++)
	{
		DealEntry &e=rec->deals[i];
		dmg+=e.chance*e.deal->GetFireDmg();
	}

	dmg*=(1.0f)/(ANIMTICK_TO_SECOND(param->gap));

	return dmg;
}


BOOL Buff_Burning::Merge(LevelRecordBuff *recNew,LevelBuffArg *arg,AnimTick dur)
{
	if (recNew)
	{
		if(recNew->GetParam<BuffParam_Burning>())
		{
			float dps=_CalcDPS(_rec);
			float dpsNew=_CalcDPS(recNew);

			if (dpsNew<=dps)
			{
				if (dur>_dur)
					_dur=dur;
				return TRUE;
			}
		}
	}

	return FALSE;
}
