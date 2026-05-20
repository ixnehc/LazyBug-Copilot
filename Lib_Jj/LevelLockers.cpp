/********************************************************************
	created:	2016/6/11 
	author:		cxi
	
	purpose:	Level Lockers
*********************************************************************/
#include "stdh.h"

#include "LevelLockers.h"


BOOL CLevelLockers::LockJumpAttack()
{
	if (_nJumpAttack>0)
		return FALSE;
	_nJumpAttack++;
	return TRUE;
}

BOOL CLevelLockers::CheckJumpAttack()
{
	if (_nJumpAttack>0)
		return FALSE;
	return TRUE;
}


void CLevelLockers::UnlockJumpAttack()
{
	_nJumpAttack=0;
}
