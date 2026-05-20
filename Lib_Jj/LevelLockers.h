#pragma once

#include "class/class.h"

#include "LevelDefines.h"





class CLevelLockers
{
public:
	CLevelLockers()
	{
		Zero();
	}
	void Zero()
	{
		_nJumpAttack=0;
	}

	BOOL LockJumpAttack();
	BOOL CheckJumpAttack();
	void UnlockJumpAttack();

protected:
	int _nJumpAttack;

};

