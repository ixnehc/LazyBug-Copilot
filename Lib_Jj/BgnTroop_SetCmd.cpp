/********************************************************************
	created:	2016/03/05 
	author:		cxi
	
	purpose:	 取消Troop的命令
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelBehavior.h"
#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelTroops.h"
#include "BgnTroop_SetCmd.h"

#include "LoUnit.h"



#include "LevelUtil.h"


////////////////////////////////////////////////////////////////////////
//CBgnTroop_SetCmd
BIND_BGN_CLASS(CBgnTroop_SetCmd,CBgpTroop_SetCmd);

void CBgnTroop_SetCmd::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_SetCmd*pad=_GetPad<CBgpTroop_SetCmd>();
	CLevel *level=_GetLevel();
	LevelBehaviorContext *ctx=_GetCtx();

	CLevelTroop *troop=_GetTroop(pad->_troop);
	if (troop)
	{
		CBgpTroop_SetCmd*pad=_GetPad<CBgpTroop_SetCmd>();
		CLevelTroop *troop=_GetTroop(pad->_troop);
		if (troop)
			troop->SetCmdToUnits(pad->_flagsRank,pad->_idCmd);
	}
	_OutputOk(outputs,1,"结束");
}

