/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnEquip.h"

#include "LevelObj.h"

#include "LoUnit.h"

////////////////////////////////////////////////////////////////////////
//CBgn_Equip
BIND_BGN_CLASS(CBgn_Equip,CBgp_Equip);

void CBgn_Equip::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Equip*pad=_GetPad<CBgp_Equip>();
	LevelBehaviorContext *ctx=_GetCtx();

	if (pad->_idItem!=RecordID_Invalid)
	{
		if ( ctx->lo)
		{
			if (ctx->lo->GetType()==LevelObjType_Unit)
			{
				if (ctx->lo->GetClass()->IsSameWith(Class_Ptr2(CLoUnit)))
				{
					CLoUnit *loUnit=(CLoUnit*)ctx->lo;
					if (!pad->_bUnEquip)
						loUnit->AddExprEquips(pad->_idItem);
					else
						loUnit->RemoveExprEquips(pad->_idItem);
				}
			}
		}
	}

	
	_OutputOk(outputs,1,"结束");
}
