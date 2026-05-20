
#include "stdh.h"

#include "Upgrade_ArmorOfVigor.h"

#include "LevelItemState.h"
#include "LevelPlayerStates.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeArmorOfVigor_Init
IMPLEMENT_ARTIFACT_UPGRADE_CLASS(CUpgradeArmorOfVigor_Init);
BOOL CUpgradeArmorOfVigor_Init::Init(LevelItemState *state)
{
	ItemBuff buff;
	buff.Set_AddMaxHP(nDeltaMaxHP);

	state->AddItemBuff(buff);

	return TRUE;
}

