/********************************************************************
	created:	2017/01/03 
	author:		cxi
	
	purpose:	随机产生Vender的位置
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelOSB.h"

#include "BgnGA_Disciple_Init.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"
#include "LoGeneralAgent.h"

#include "LevelTroops.h"


#include "Log/LogDump.h"
#include "Random/Random.h"

IMPLEMENT_CLASS(BMO_DiscipleData);


////////////////////////////////////////////////////////////////////////
//CBgnGA_Diciple_Init
BIND_BGN_CLASS(CBgnGA_Diciple_Init,CBgpGA_Diciple_Init);

void CBgnGA_Diciple_Init::Start(DWORD iStb,BGNOutputs &outputs)
{
	Update(outputs);
	return;
}


void CBgnGA_Diciple_Init::Update(BGNOutputs &outputs)
{
	CBgpGA_Diciple_Init*pad=_GetPad<CBgpGA_Diciple_Init>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	CLevelTroop *troop=_GetTroop(pad->troop);
	DWORD c=	troop->GetFrameCount();

	BOOL bDone=FALSE;
	for (int i=0;i<c;i++)
	{
		LevelTroopFrame *fr=troop->GetFrame(i);
		if (fr)
		{
			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			CLevelObj *lo=LevelUtil_GetAliveLo(level,fr->idUnit);
			if (lo)
			{
				CLevelBehavior *bhv=lo->GetBehaviorAI();
				if (bhv)
				{
					CBehaviorMem *mem=bhv->GetMem(0);
					if (mem)
					{
						if (pad->nmVar!=StringID_Invalid)
						{
							BMO_DiscipleData *data=Class_New(BMO_DiscipleData);
							data->param=pad->param;
							mem->DepositObj(pad->nmVar,data);
							bDone=TRUE;
						}
						break;
					}
				}
			}
		}
	}

	if (bDone)
		_OutputOk(outputs,1,"结束");

}
