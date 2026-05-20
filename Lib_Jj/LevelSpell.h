#pragma once

#include "gds/GObj.h"

#include "LevelAbility.h"

#include "BgnGA_RollAwards.h"


class CLevelSpellInitial:public CLevelAbilityInitial
{
public:


};


class CLevelSpell:public CLevelAbility
{
public:
	virtual BOOL IsSpell()	{		return TRUE;	}

};

