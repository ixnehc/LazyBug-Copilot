/********************************************************************
	created:	2022/03/13 
	author:		cxi
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "LoUnit.h"

#include "BgnCheckMap.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckMap
BIND_BGN_CLASS(CBgn_CheckMap,CBgp_CheckMap);
void CBgn_CheckMap::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckMap*pad=_GetPad<CBgp_CheckMap>();

	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	if (level->GetMapID()==pad->idMap)
	{
		_OutputOk(outputs,1,"是");
		return;
	}

	_OutputFail(outputs,2,"否");
}
