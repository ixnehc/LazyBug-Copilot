/********************************************************************
	created:	2022/3/2 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
  
#include "BgnTroop_Quit.h"

#include "LevelObj.h"
#include "LevelBGs.h"
#include "LevelTroops.h"

////////////////////////////////////////////////////////////////////////
//CBgnTroop_Quit
BIND_BGN_CLASS(CBgnTroop_Quit,CBgpTroop_Quit);


void CBgnTroop_Quit::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_Quit*pad=_GetPad<CBgpTroop_Quit>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	CLevelTroop *troop=lo->GetTroop();
	if (troop)
		troop->DetachUnit(lo->GetID());

	_OutputOk(outputs,1,"结束");
}
