
#include "stdh.h"

#include "Level.h"


#include "LoLifeFlies.h"

#include "Random/Random.h"

#include "LevelRecords.h"


BOOL CLoLifeFlies::OnActivate()
{
	LopLifeFlies *param=(LopLifeFlies *)_param;

	_amntSleep=param->amnt;
	_amntFly=0;

	return TRUE;
}


void CLoLifeFlies::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Bits_Write(_amntSleep,5);
	bp->Bits_Write(_amntFly,5);
	bContent=TRUE;
}

