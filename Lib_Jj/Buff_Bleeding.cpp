/********************************************************************
	created:	2016/06/24 
	author:		cxi
	
	purpose:	BleedingµÄBuff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"


#include "Buff_Bleeding.h"



//////////////////////////////////////////////////////////////////////////
//CBuff_Bleeding
BIND_BUFFPARAM(Buff_Bleeding,BuffParam_Bleeding,BuffArg_Bleeding);

LevelBuffMask Buff_Bleeding::GetForbiddingBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Bleeding)->GetUID());

	return mask;
}

void Buff_Bleeding::_OnUpdate(AnimTick dt)
{
	BuffParam_Bleeding *param=(BuffParam_Bleeding*)_param;
	int nToDeal=_tAge/param->gap;

	DealArg arg;
	CLevelObj *lo=_GetOwner();
	while (nToDeal>_nDeal)
	{
		_nDeal++;

		int nToDamage=(int)(param->nDmgPerSec*ANIMTICK_TO_SECOND(_tAge));
		if (nToDamage>_nDamaged)
		{
			CLevel *level=_GetLevel();
			if (level)
			{
				int nDelta=_nDamaged-nToDamage;
				LevelStrike strike;
				int nMod;
				level->GetDecider()->CommitHPMod(nDelta,LevelOSB(_GetOwner()),_GetOwner(),strike,LevelOpLink(),nMod);
			}

			_nDamaged=nToDamage;
		}
	}
}

