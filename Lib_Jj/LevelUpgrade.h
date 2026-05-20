#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "anim/animdefines.h"

#include "LevelDefines.h"

class CLevelRecords;
class CLevelUpgrade
{
public:
	enum Type
	{
		None,
		Ability,
		Artifact,
	};

	virtual CClass *GetClass()=0;
	virtual GObjBase*GetGObj()=0;

	virtual Type GetUpgradeType()=0;

	virtual const char *GetDesc(LevelAwardSeed &seed,CLevelRecords *records)=0;
};


class CUpgrade_Void:public CLevelUpgrade
{
public:
	DEFINE_CLASS(CUpgrade_Void);

	BEGIN_GOBJ_PURE(CUpgrade_Void,1);
	END_GOBJ();

	virtual CLevelUpgrade::Type GetUpgradeType()
	{
		return CLevelUpgrade::None;
	}
	virtual const char *GetDesc(LevelAwardSeed &seed,CLevelRecords *records)
	{
		return "";
	}

};
