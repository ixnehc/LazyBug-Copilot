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
#include "LevelAIContext.h"

#include "BgnAI_MonitorCmd.h"

#include "Log/LogDump.h"

////////////////////////////////////////////////////////////////////////
//CBgnCmd_Monitor
BIND_BGN_CLASS(CBgnCmd_Monitor,CBgpCmd_Monitor);

void CBgnCmd_Monitor::Destroy()
{
}

void CBgnCmd_Monitor::Start(DWORD iStb,BGNOutputs &outputs)
{
	Update(outputs);
}

void CBgnCmd_Monitor::Update(BGNOutputs &outputs)
{
	CBgpCmd_Monitor*pad=_GetPad<CBgpCmd_Monitor>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	if (lo)
	{
		StringID idCmd=lo->GetAICmd();
		if (idCmd!=_idCurCmd)
		{
			_idCurCmd=idCmd;
			int iStb=-1;
			for (int i=0;i<pad->_cmds.size();i++)
			{
				if (pad->_cmds[i]==_idCurCmd)
				{
					iStb=i+1;
					break;
				}
			}

			if (iStb>=0)
			{
				BgnThread thrd;
				thrd=_thrd;
				thrd.idNode=_id;
				thrd.keyRewind=BGNTHREAD_INVALID_REWINDKEY;
				outputs.Add(iStb,thrd);
			}
		}
	}
}

void CBgnCmd_Monitor::Break(BGNOutputs &outputs)
{
	BgnThread thrd;
	thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=BGNTHREAD_INVALID_REWINDKEY;
	outputs.thrdsBreak.push_back(thrd);
}
