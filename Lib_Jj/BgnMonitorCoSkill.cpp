/********************************************************************
	created:	2013/6/20 
	author:		cxi
	
	purpose:	监控是否有随从命令
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "Level.h"

#include "BgnMonitorCoSkill.h"

#include "LoUnit.h"
#include "LevelCoSkill.h"

////////////////////////////////////////////////////////////////////////
//CBgn_MonitorCoSkill
BIND_BGN_CLASS(CBgn_MonitorCoSkill,CBgp_MonitorCoSkill);
void CBgn_MonitorCoSkill::Start(DWORD iStb,BGNOutputs &outputs)
{
	Update(outputs);
}

void CBgn_MonitorCoSkill::Update(BGNOutputs &outputs)
{
	CBgp_MonitorCoSkill*pad=_GetPad<CBgp_MonitorCoSkill>();

	CLevelObj *lo=_GetLo();

	if (lo)
	{
		if (lo->IsRetinue())
		{
			CLevelCoSkill *coskill=lo->ObtainCoSkill();
			if (coskill)
			{
				for (int i=0;i<pad->coskills.size();i++)
				{
					RecordID idSkill=pad->coskills[i];
					if (coskill->FetchCharge(idSkill)==idSkill)
					{
						outputs.Add(1+i,_thrd);
						_SetResult(A_Ok);
						return;
					}
				}
			}
		}
	}


}
