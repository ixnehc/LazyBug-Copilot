
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoDemonBlood.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoDemonBlood,EoParamDemonBlood);

void EoDemonBlood::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Data_WriteSimple(_arg);
}


void EoDemonBlood::Init(EoDemonBloodArg &arg)
{
	_arg=arg;

}


void EoDemonBlood::_OnUpdate()
{
	EoParamDemonBlood *param=GetParam<EoParamDemonBlood>();
	if (!param)
		return;

// 	if (_level->GetT_()-_tCreate>ANIMTICK_FROM_SECOND(5.0f))
// 		DeferDestroy();
}

// 
// void EoDemonBlood::Confirm()
// {
// 	CLevelObj *loTarget=_level->GetIDs()->LoFromID(_arg.idTarget);
// 	if (loTarget)
// 	{
// 		if (loTarget->IsAlive())
// 		{
// 			DealArg arg;
// 			arg.dir.set(0,0,0);
// 			arg.grd=1;
// 			arg.amount=1;
// 
// 			_MakeDeals(loTarget,arg);
// 		}
// 	}
// 
// }
