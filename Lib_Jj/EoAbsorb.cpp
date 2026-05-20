
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoAbsorb.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoAbsorb,EoParamAbsorb);

void EoAbsorb::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Data_WriteSimple(_arg);
}


void EoAbsorb::Init(EoAbsorbArg &arg)
{
	_arg=arg;

}


void EoAbsorb::_OnUpdate()
{
	EoParamAbsorb *param=GetParam<EoParamAbsorb>();
	if (!param)
		return;

	if (_level->GetT_()-_tCreate>ANIMTICK_FROM_SECOND(5.0f))
		DeferDestroy();
}


void EoAbsorb::Confirm()
{
	CLevelObj *loTarget=_level->GetIDs()->LoFromID(_arg.idTarget);
	if (loTarget)
	{
		if (loTarget->IsAlive())
		{
			DealArg arg;
			arg.dir.set(0,0,0);
			arg.grd=1;
			arg.amount=1;

			_MakeDeals(loTarget,arg);
		}
	}

}
