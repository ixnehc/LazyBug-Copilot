/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelBehavior.h"
#include "BgnSetCollide.h"

#include "LevelObj.h"


////////////////////////////////////////////////////////////////////////
//CBgn_SetCollide
BIND_BGN_CLASS(CBgn_SetCollide,CBgp_SetCollide);
void CBgn_SetCollide::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		CUnit *unit=lo->GetUnit();
		if (unit)
		{
			extern void UnitCollide_SetGhost(CUnit *unit,BOOL bGhost);
			CBgp_SetCollide*pad=_GetPad<CBgp_SetCollide>();
			if (pad->_op==CBgp_SetCollide::SetGhost)
				UnitCollide_SetGhost(unit,TRUE);
			if (pad->_op==CBgp_SetCollide::ClearGhost)
				UnitCollide_SetGhost(unit,FALSE);
		}
	}

	_OutputOk(outputs,1,"结束");
}
