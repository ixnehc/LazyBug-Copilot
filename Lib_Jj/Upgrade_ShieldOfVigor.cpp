
#include "stdh.h"

#include "Upgrade_ShieldOfVigor.h"

#include "LevelItemState.h"
#include "LevelPlayerStates.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeShieldOfVigor_Init
IMPLEMENT_ARTIFACT_UPGRADE_CLASS(CUpgradeShieldOfVigor_Init);
BOOL CUpgradeShieldOfVigor_Init::Init(LevelItemState *state)
{
	ItemBuff buff;
	buff.Set_AddMaxHP(nDeltaMaxHP);

	state->AddItemBuff(buff);

	return TRUE;
}

