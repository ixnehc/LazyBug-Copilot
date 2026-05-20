/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"


#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "BgnDestroyLo.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DestroyLo

extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);

BIND_BGN_CLASS(CBgn_DestroyLo,CBgp_DestroyLo);
void CBgn_DestroyLo::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_DestroyLo*pad=_GetPad<CBgp_DestroyLo>();

	CLevelObj *lo=NULL;
	if (pad->_nmLo!=StringID_Invalid)
	{
		LevelObjID idLo;
		if (_GetID(pad->_nmLo,BehaviorMemType_ObjID,idLo))
			lo=LevelUtil_GetAliveLo(_GetLevel(),idLo);
	}
	else
		lo=_GetLo();

	lo->DeferDestroy();
	_SetResult(A_Ok);
}
