/********************************************************************
	created:	2023/10/05 
	author:		cxi
	
	purpose:	 BellyEggµÄEO
*********************************************************************/

#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"
#include "LoBelly.h"

#include "LevelUtil.h"

#include "EoBellyEgg.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoBellyEgg,EoParamBellyEgg);

void EoBellyEgg::_OnPostCreate()
{
	CLoBelly *loBelly=(CLoBelly *)_level->GetUniqueObj(LevelUniqueObj_Belly);
	if (loBelly)
		loBelly->RegisterEgg(GetID());
}


void EoBellyEgg::_OnDetroy()
{
	CLoBelly *loBelly=(CLoBelly *)_level->GetUniqueObj(LevelUniqueObj_Belly);
	if (loBelly)
		loBelly->UnregisterEgg(GetID());
}


void EoBellyEgg::_OnUpdate()
{
	EoParamBellyEgg *param=GetParam<EoParamBellyEgg>();
	if (!param)
		return;

	if (!_bTriggered)
	{
		CLevelObj *loPlayer=LevelUtil_DetectClosestPlayer(this,0.5f);
		if (loPlayer)
			Trigger(LevelOpLink(),loPlayer);
	}


}

void EoBellyEgg::Trigger(LevelOpLink &link,CLevelObj *loPlayer)
{
	if (_bTriggered)
		return;
	_bTriggered=TRUE;

	EoParamBellyEgg *param=GetParam<EoParamBellyEgg>();

	LevelOp_StartFire *op=NewOp<LevelOp_StartFire>(link);
	AddOp(op);

	DealArg arg;
	arg.link=link;
	if (!loPlayer)
		_MakeDeals(GetFramePos3D(),arg);
	else
	{
		MakeDeals(param->dealsFromPlayer,LevelOSB(loPlayer),GetFramePos3D(),arg,NULL);
	}

	CLoBelly *loBelly=(CLoBelly *)_level->GetUniqueObj(LevelUniqueObj_Belly);
	if (loBelly)
		loBelly->UnregisterEgg(GetID());


}
