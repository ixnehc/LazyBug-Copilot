/********************************************************************
	created:	2014/10/0 5
	author:		cxi
	
	purpose:	unit group 操作
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "Level.h"

#include "LevelTroops.h"

#include "BgnTroop_Clean.h"

#include "Log/LogDump.h"

////////////////////////////////////////////////////////////////////////
//CBgnTroop_Clean
BIND_BGN_CLASS(CBgnTroop_Clean,CBgpTroop_Clean);
void CBgnTroop_Clean::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_Clean*pad=_GetPad<CBgpTroop_Clean>();

	CLevel *level=_GetLevel();
	CLevelTroop *troop=_GetTroop(pad->_troop);

	if (troop)
	{
		if (pad->_tp==CBgpTroop_Clean::CleanAll)
		{
			troop->DetachAllUnits(TRUE);
			troop->FlushDeadFrames();
		}
		if (pad->_tp==CBgpTroop_Clean::FlushDeadFrame)
		{
			troop->FlushDeadFrames();
		}
		if (pad->_tp==CBgpTroop_Clean::Detach)
		{
			troop->ClearUnitsAndFrames(FALSE);
		}
	}
	_OutputOk(outputs,1,"OK");
	return;
}

