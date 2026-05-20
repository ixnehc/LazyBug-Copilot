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

#include "BgnRevive.h"

#include "Buff_Dormant.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Revive
BIND_BGN_CLASS(CBgn_Revive,CBgp_Revive);


void CBgn_Revive::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Revive*pad=_GetPad<CBgp_Revive>();
	LevelBehaviorContext *ctx=_GetCtx();

	extern CLevelBuff *LevelUtil_FindBuff(CLevelObj *lo,CClass *clssBuff);
	Buff_Dormant *buff=(Buff_Dormant *)LevelUtil_FindBuff(ctx->lo,Class_Ptr2(Buff_Dormant));
	if (buff)
	{
		LevelObjID idTarget=LevelObjID_Invalid;
		if (pad->_nmVar!=StringID_Invalid)
			_GetID(pad->_nmVar,BehaviorMemType_ObjID,idTarget);
		buff->Revive(idTarget);
	}

	_OutputOk(outputs,1,"结束");
}
