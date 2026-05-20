/********************************************************************
	created:	2019/09/14 
	author:		cxi
	
*********************************************************************/

#include "stdh.h"
#include "LevelBlocking.h"

#include "LevelRecordUnit.h"

#include "commondefines/general_stl.h"

void CLevelBlocking::Activate()
{
	_bActive=TRUE;
	_tActivated=_owner->GetT();
}



BOOL CLevelBlocking::CanBlock(LevelPos dir,AnimTick t)
{
	if (_bActive)
		return TRUE;
	return FALSE;
}

void CLevelBlocking::AddBlock(LevelPos dir,AnimTick t)
{

}
