/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelObj.h"

#include "BgnRaiseUp.h"

#include "Level.h"
#include "LevelRecords.h"

#include "LevelBGs.h"


#include "LevelRecordBuff.h"
#include "LevelBuff.h"

#include "Buff_KD.h"

////////////////////////////////////////////////////////////////////////
//CBgn_RaiseUp
BIND_BGN_CLASS(CBgn_RaiseUp,CBgp_RaiseUp);
void CBgn_RaiseUp::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_RaiseUp*pad=_GetPad<CBgp_RaiseUp>();
	CLevel *level=_GetLevel();

	if (pad->_idBuff!=RecordID_Invalid)
	{
		CLevelBuffs *buffs=_GetBuffs();
		if (buffs)
		{
			CLevelBuff *buff=buffs->FindBuffByRecordID(pad->_idBuff);
			if (buff->GetClass()->IsSameWith(Class_Ptr2(Buff_KD)))
			{
				((Buff_KD*)buff)->RaiseUp();
				_idBuff=buff->GetID();
			}
		}
	}

}

void CBgn_RaiseUp::Update(BGNOutputs &outputs)
{
	BOOL bCanFinish=TRUE;
	if (_idBuff!=LevelBuffID_Invalid)
	{
		CLevelBuffs *buffs=_GetBuffs();
		if (buffs)
		{
			if (buffs->FindBuffByID(_idBuff))
				bCanFinish=FALSE;//还未结束
		}
	}

	if (bCanFinish)
	{
		_OutputOk(outputs,1,"结束");
	}

}
