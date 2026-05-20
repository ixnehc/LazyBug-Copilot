/********************************************************************
	created:	2013/6/20 
	author:		cxi
	
	purpose:	监控是否有随从命令
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "Level.h"

#include "LevelTroops.h"

#include "BgnTroop_Monitor.h"

#include "Log/LogDump.h"

////////////////////////////////////////////////////////////////////////
//CBgnTroop_Monitor
BIND_BGN_CLASS(CBgnTroop_Monitor,CBgpTroop_Monitor);
void CBgnTroop_Monitor::Start(DWORD iStb,BGNOutputs &outputs)
{
	Update(outputs);
}

void CBgnTroop_Monitor::Update(BGNOutputs &outputs)
{
	CBgpTroop_Monitor*pad=_GetPad<CBgpTroop_Monitor>();

	CLevel *level=_GetLevel();
	CLevelTroop *troop=_GetTroop(pad->_troop);

	if (troop)
	{
		switch(pad->_tp)
		{
			case CBgpTroop_Monitor::AllDead:
			{
				if (troop->IsAllDead(pad->_flagsRank))
				{
					_OutputOk(outputs,1,"OK");
					return;
				}
				break;
			}
			case CBgpTroop_Monitor::HPRatio_AnyBelow:
			{
				if (troop->CheckHealthRatio_AnyBelow(pad->_flagsRank,pad->_ratioHP))
				{
					_OutputOk(outputs,1,"OK");
					return;
				}
				break;
			}
		}
	}

}
