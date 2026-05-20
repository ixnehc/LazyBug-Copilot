#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

#include <unordered_set>


class CUpgradeHonorBook_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeHonorBook_Init,LevelAbilityType_HonorBook);

	BEGIN_GOBJ_PURE(CUpgradeHonorBook_Init,1);


	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:



	friend class CLevelAbility_HonorBook;

};



struct CBHonorBookThrust;
struct LevelRecordSkill;
class CLevelAbility_HonorBook:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_HonorBook,CUpgradeHonorBook_Init,LevelAbilityType_HonorBook);

	CLevelAbility_HonorBook()
	{
		GConstructor();
	}
	~CLevelAbility_HonorBook();


	BEGIN_GOBJ_UID(CLevelAbility_HonorBook,1);

		GELEM_ABILITY_BASE();
	END_GOBJ();

public:

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnBuildArtifactState(LevelItemState &state)	 override;

	virtual void _OnEvent(LevelEvent &e) override;

public://Take it as protected

	virtual void _InitTechs()	 override{	}

public:


};


