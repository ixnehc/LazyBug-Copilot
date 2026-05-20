
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoGeneralAgent.h"

#include "EoUtumRepairBridge.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "Random/Random.h"
#include "timer/timer.h"

#include "behaviorgraph/BehaviorMem.h"



//////////////////////////////////////////////////////////////////////////
//EoUtumRepairBridge
BIND_EOPARAM(EoUtumRepairBridge,EoParamUtumRepairBridge);


void EoUtumRepairBridge::_OnPostCreate()
{
	EoParamUtumRepairBridge *param=GetParam<EoParamUtumRepairBridge>();

}

void EoUtumRepairBridge::OnDestroy()
{
}


void EoUtumRepairBridge::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Data_WriteSimple(_idBridge);
	bp->Data_WriteSimple(_idxSite);
	bContent=TRUE;
}

void EoUtumRepairBridge::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
}

void EoUtumRepairBridge::_OnPostWriteSync()
{
}


void EoUtumRepairBridge::_OnUpdate()
{
	EoParamUtumRepairBridge *param=GetParam<EoParamUtumRepairBridge>();

	if (!_bRepaired)
	{
		CLevelObj *loBridge=LevelUtil_GetAliveLo(_level,_idBridge);
		if (loBridge)
		{
			if (loBridge->GetClass()->IsSameWith(Class_Ptr2(CLoGeneralAgent)))
			{
				CLevelBehavior*bhv=((CLoGeneralAgent*)loBridge)->GetBehavior(LevelPlayerID(0));
				if (bhv)
				{
					CBehaviorMem *mem=bhv->GetMem(0);
					if (mem)
					{
						if (param->nmBridgeStateVar!=StringID_Invalid)
						{
							StringID idStr;
							if (mem->GetID(param->nmBridgeStateVar,BehaviorMemType_StringID,idStr))
							{
								if (GetRepairedStr()==StrLib_GetStr(idStr))
								{
									_bRepaired=TRUE;
									_tRepaired=_GetT();
								}
							}
						}
					}
				}
			}
		}
	}

	if (_GetAge()>ANIMTICK_FROM_SECOND(20.0f))
		DeferDestroy();

}

