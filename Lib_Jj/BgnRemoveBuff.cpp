/********************************************************************
	created:	2016/05/16 
	author:		cxi
	
	purpose:	 清除某个Buff
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelRecordItemClass.h"

#include "LevelOSB.h"

#include "BgnRemoveBuff.h"


BIND_BGN_CLASS(CBgn_RemoveBuff,CBgp_RemoveBuff);

void CBgn_RemoveBuff::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_RemoveBuff*pad=_GetPad<CBgp_RemoveBuff>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	if (pad->_tpTarget==1)
		lo=_GetTalkLo();

	if (lo)
	{
		extern void LevelUtil_RemoveBuffByRecordID(CLevelObj *lo,RecordID idBuff);
		LevelUtil_RemoveBuffByRecordID(lo,pad->_idBuff);
	}

	_OutputOk(outputs,1,"结束");
}

