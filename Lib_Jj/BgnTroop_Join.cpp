/********************************************************************
	created:	2019/12/19 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
  
#include "BgnTroop_Join.h"

#include "LevelObj.h"
#include "LevelBGs.h"
#include "LevelTroops.h"

////////////////////////////////////////////////////////////////////////
//CBgnTroop_Join
BIND_BGN_CLASS(CBgnTroop_Join,CBgpTroop_Join);


void CBgnTroop_Join::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_Join*pad=_GetPad<CBgpTroop_Join>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelObjID idTroopOwner=LevelObjID_Invalid;
	_GetID(pad->_idTroopOwner,BehaviorMemType_ObjID,idTroopOwner);

	extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
	CLevelObj *loTroopOwner=LevelUtil_GetAliveLo(level,idTroopOwner);
	if (loTroopOwner)
	{
		CLevelTroops *troops=loTroopOwner->GetTroops();

		CLevelTroop *troop=NULL;
		if (pad->_troop!=StringID_Invalid)
			troop=troops->Get(pad->_troop);
		else
			troop=troops->GetFirst();

		if (troop)
		{
			LevelObjID idUnit=lo->GetID();
			if (pad->_idUnit!=StringID_Invalid)
				_GetID(pad->_idUnit,BehaviorMemType_ObjID,idUnit);
			troop->AddUnit(pad->_rank,idUnit);
		}
	}

	_OutputOk(outputs,1,"结束");
}
