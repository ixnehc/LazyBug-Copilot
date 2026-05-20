/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelObj.h"

#include "BgnKnockDown.h"

#include "Level.h"
#include "LevelBGs.h"

#include "LevelRecords.h"

#include "LevelRecordBuff.h"
#include "LevelBuff.h"

#include "Buff_KD.h"

////////////////////////////////////////////////////////////////////////
//CBgn_KnockDown
BIND_BGN_CLASS(CBgn_KnockDown,CBgp_KnockDown);
void CBgn_KnockDown::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelSkillDriver *driver=_GetSkillDriver();
	if (driver)
		driver->StopMove();//先暂停移动,然后等下一帧再加Buff(我们要确保稳定的不移动后,才加Buff)
}

void CBgn_KnockDown::Update(BGNOutputs &outputs)
{
	CBgp_KnockDown*pad=_GetPad<CBgp_KnockDown>();
	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	if (pad->_idBuff!=RecordID_Invalid)
	{
		LevelRecordBuff *recBuff=level->GetRecords()->GetBuff(pad->_idBuff);
		if (recBuff)
		{
			if (recBuff->param->GetClass()->IsSameWith(Class_Ptr2(BuffParam_KD)))
			{
				BuffArg_KD arg;
				if (lo)
				{
					level->GetDecider()->MakeBuff(lo,pad->_idBuff,ANIMTICK_INFINITE,&arg,TRUE);
				}
			}
		}
	}
	_OutputOk(outputs,1,"结束");
}
