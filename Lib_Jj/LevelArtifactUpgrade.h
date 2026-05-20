#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelItemState;
class CLevelArtifactUpgrade:public CLevelUpgrade
{
public:
	virtual CLevelUpgrade::Type GetUpgradeType()	{		return CLevelUpgrade::Artifact;	}

	virtual LevelArtifactType GetArtifactType()=0;
	virtual BOOL CanUpgrade(LevelItemState*state)=0;
	virtual void MakeSeed(LevelItemState *state,LevelAwardSeed &seed)=0;
	virtual void Upgrade(LevelItemState *state,LevelAwardSeed &seed)=0;

	virtual BOOL Init(LevelItemState *state)	{		return FALSE;	}
};

class CLevelArtifactUpgrade_Initial:public CLevelArtifactUpgrade
{
public:
	virtual BOOL CanUpgrade(LevelItemState *state)	{		return FALSE;	}
	virtual void MakeSeed(LevelItemState *state,LevelAwardSeed &seed)	{	}
	virtual void Upgrade(LevelItemState *state,LevelAwardSeed &seed)	{	}
	virtual const char *GetDesc(LevelAwardSeed &seed,CLevelRecords *records)	{		return "";	}

};

#define DEFINE_ARTIFACT_UPGRADE_CLASS(clss,tpArtifact)											\
	DEFINE_CLASS(clss);																					\
	virtual LevelArtifactType GetArtifactType()											\
	{																														\
		return tpArtifact;																			\
	}


#define IMPLEMENT_ARTIFACT_UPGRADE_CLASS(clss)													\
	CClass *GetClass_##clss()														\
	{																													\
		return Class_Ptr2(clss);															\
	}
