/********************************************************************
	created:	2013/01/25 
	author:		cxi
	
	purpose:	 检查HP的范围
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "LevelBehavior.h"

#include "BgnGetLevelObjID.h"

////////////////////////////////////////////////////////////////////////
//CBgn_GetLo

BIND_BGN_CLASS(CBgn_GetLevelObjID,CBgp_GetLevelObjID);

void CBgn_GetLevelObjID::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_GetLevelObjID*pad=_GetPad<CBgp_GetLevelObjID>();

	if (pad->nmLo!=StringID_Invalid)
	{
		CLevelObj *lo=NULL;
		if (pad->tp==CBgp_GetLevelObjID::TalkPlayer)
			lo=_GetTalkLo();
		if (pad->tp==CBgp_GetLevelObjID::Me)
			lo=_GetLo();
		if (pad->tp==CBgp_GetLevelObjID::LockPlayer)
			lo=_GetLockLo();
		if (lo)
		{
			if (lo->IsAlive())
			{
				if (TRUE==_SetID(pad->nmLo,BehaviorMemType_ObjID,lo->GetID()))
				{
					_OutputOk(outputs,1,"成功");
					return;
				}
			}
		}
	}

	_OutputFail(outputs,2,"失败");
}


