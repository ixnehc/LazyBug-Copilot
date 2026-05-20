/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelRecordItemClass.h"
 
#include "BgnCheckResiding.h"  

#include "Buff_ResideHole.h"
#include "Buff_ResideWT.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckResiding

BIND_BGN_CLASS(CBgn_CheckResiding,CBgp_CheckResiding);

void CBgn_CheckResiding::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckResiding*pad=_GetPad<CBgp_CheckResiding>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		CLevelBuffs *buffs=lo->GetBuffs();
		if (buffs)
		{
			LevelObjID idTarget=LevelObjID_Invalid;
			Buff_ResideWT *buffWT=(Buff_ResideWT *)buffs->FindBuff(Class_Ptr2(Buff_ResideWT));
			if (buffWT)
				idTarget=buffWT->GetResidingTarget();
			else
			{
				Buff_ResideHole*buffHole=(Buff_ResideHole*)buffs->FindBuff(Class_Ptr2(Buff_ResideHole));
				if (buffHole)
					idTarget=buffHole->GetResidingTarget();
			}

			if (idTarget!=LevelObjID_Invalid)
			{
				CLevelObj *loTarget=level->GetIDs()->LoFromID(idTarget);
				extern RecordID LevelUtil_GetAgentRecID(CLevelObj *lo);
				if (pad->_idAgent==LevelUtil_GetAgentRecID(loTarget))
				{
					_OutputOk(outputs,1,"驻留");
					return;
				}
			}
		}
	}

	_OutputOk(outputs,2,"未驻留");
}

