#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"
#include "LevelSlateDefines.h"

#include "LevelSlatesBasis.h"

#include "LevelChancer.h"

#include "LevelSensor.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"
#include "LevelObjResidable.h"

#include "LevelAttrs.h"

#include "LevelBuff.h"

#include "LevelSkillDriver.h"

struct SlatesRandomPickEntry
{
	LevelSlateType tp;
	int count;
	std::vector<LevelSlateType> tpsTarget;
};

struct LopSlates:public CLevelObjParam
{
public:

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

	std::vector<LevelGUID> includes;//AssetUID类型

	SlateSpaceDefine space;

	std::vector<SlateGrpEntry> grps;

	StringID bhvSetup;


};

struct LosSlates:public LosAgent
{
public:

	virtual BOOL NeedSyncGUID()override	{		return TRUE;	}


};

class CLevelSensor_Slates:public CLevelSensor
{
public:
	void SetThreat(CLevelObj *threat)
	{
		SAFE_REPLACE(_threat,threat);
	}


};
