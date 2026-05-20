/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 苏醒
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnInvisible.h"

#include "Buff_Invisible.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Invisible
BIND_BGN_CLASS(CBgn_Invisible,CBgp_Invisible);


void CBgn_Invisible::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Invisible*pad=_GetPad<CBgp_Invisible>();

	CLevelObj *lo=_GetLo();
	if (lo)
	{
		if (pad->_idBuff!=RecordID_Invalid)
		{
			BuffArg_Invisible arg;
			lo->GetLevel()->GetDecider()->MakeBuff(lo,pad->_idBuff,ANIMTICK_INFINITE,&arg,TRUE);
		}
	}

	_OutputOk(outputs,1,"结束");
}
