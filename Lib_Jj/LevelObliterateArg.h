#pragma once

#include "class/class.h"

#include "LevelDefines.h"

#include "LevelAttrs_DamageAttr.h"

enum LevelObliterateType
{
	LevelObliterate_None,
	LevelObliterate_Blood,

	LevelObliterate_ForceDword=0xffffffff,
};
#define LevelObliterateType_SemConstraint "n/a,血爆"

struct LevelObliterateArg
{
	LevelObliterateArg()
	{
		tp=LevelObliterate_None;
	}
	LevelObliterateType tp;
	LevelAttr_Damages dmgs;
};
