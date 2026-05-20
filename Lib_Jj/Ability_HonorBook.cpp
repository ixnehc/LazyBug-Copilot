
#include "stdh.h"

#include "Protocal.h"

#include "Ability_HonorBook.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelRecordEO.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeHonorBook_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeHonorBook_Init);
BOOL CUpgradeHonorBook_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_HonorBook *ability=(CLevelAbility_HonorBook *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_HonorBook

CLevelAbility_HonorBook::~CLevelAbility_HonorBook()
{
	GDestructor();
}


void CLevelAbility_HonorBook::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_HonorBook::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_HonorBook::_SaveSync(CDataPacket &dp)
{
}

void CLevelAbility_HonorBook::_LoadSync(CDataPacket &dp,CRecords *records)
{
}


void CLevelAbility_HonorBook::_OnBuildArtifactState(LevelItemState &state)
{
}

void CLevelAbility_HonorBook::_OnEvent(LevelEvent &e0)
{
	CUpgradeHonorBook_Init *upgradeInitial=(CUpgradeHonorBook_Init*)_upgradeInitial;

	if (e0.GetType()==LET_Kill)
	{
		LeKill &e=(LeKill &)e0;	
		if (e.osbSrc)
		{
			if (_owner)
			{
				LevelObjID id=e.osbSrc->GetRootOwnerID();
				if (id==_owner->GetID())
				{
					extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
					CLevelPlayer *player=LevelUtil_PlayerFromLo(_owner);
					if (player)
					{
						LPS_AddKilling(player->GetLPS(),e.loTarget);
						LPS_IncKillingHonor(player->GetLPS(),e.loTarget);
					}
				}

			}
		}
	}

}

