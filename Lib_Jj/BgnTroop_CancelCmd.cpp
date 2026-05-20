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
#include "BgnTroop_CancelCmd.h"

#include "LoUnit.h"



#include "LevelUtil.h"


////////////////////////////////////////////////////////////////////////
//CBgnTroop_CancelCmd
BIND_BGN_CLASS(CBgnTroop_CancelCmd,CBgpTroop_CancelCmd);

void CBgnTroop_CancelCmd::Destroy()
{
	_DiscardTroopControl();
}


void CBgnTroop_CancelCmd::_DiscardTroopControl()
{
	CBgpTroop_CancelCmd*pad=_GetPad<CBgpTroop_CancelCmd>();
	CLevelTroop *troop=_GetTroop(pad->_troop);
	if (troop)
	{
		troop->SetCmdToUnits(pad->_flagsRank,StringID_Invalid);
	}
}



void CBgnTroop_CancelCmd::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_CancelCmd*pad=_GetPad<CBgpTroop_CancelCmd>();
	CLevel *level=_GetLevel();
	LevelBehaviorContext *ctx=_GetCtx();

	CLevelTroop *troop=_GetTroop(pad->_troop);
	if (troop)
	{
		//接管所有troop单位的控制权
		_DiscardTroopControl();

		return;
	}
	_OutputFail(outputs,1,"失败");
}


void CBgnTroop_CancelCmd::Update(BGNOutputs &outputs)
{
	CBgpTroop_CancelCmd*pad=_GetPad<CBgpTroop_CancelCmd>();

	CLevelTroop *troop=_GetTroop(pad->_troop);
	if (!troop)
	{
		_OutputFail(outputs,1,"失败");
		return;
	}

	_DiscardTroopControl();

}

