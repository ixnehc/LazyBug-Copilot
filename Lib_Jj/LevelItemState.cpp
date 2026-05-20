
#include "stdh.h"

#include "LevelItemState.h"
#include "LevelItemBuffType.h"

//////////////////////////////////////////////////////////////////////////
//ItemBuff

void ItemBuff::Set_AddMaxHP(int delta)
{
	tp=(ItemBuffType)ItemBuff_AddMaxHP;
	sh=delta;
}

void ItemBuff::Set_AddFullSP(int delta)
{
	tp=(ItemBuffType)ItemBuff_AddFullSP;
	sh=delta;
}

void ItemBuff::Set_AddPhysDef(int delta)
{
	tp=(ItemBuffType)ItemBuff_PhysDef;
	sh=delta;
}

void ItemBuff::Set_AddPhysDef_Rate(int delta)
{
	tp=(ItemBuffType)ItemBuff_PhysDefRate;
	sh=delta;
}


void ItemBuff::Set_AddPhysDef_Base(int delta)
{
	tp=(ItemBuffType)ItemBuff_PhysDef_Base;
	sh=delta;
}

void ItemBuff::Set_AddMoveSpeed(int delta)
{
	tp=(ItemBuffType)ItemBuff_MoveSpeed;
	sh=delta;
}

void ItemBuff::Set_AddAttackSpeed(int delta)
{
	tp=(ItemBuffType)ItemBuff_AttackSpeed;
	sh=delta;
}

void ItemBuff::Set_AddAttackSpeedBow(int delta)
{
	tp=(ItemBuffType)ItemBuff_AttackSpeed_Bow;
	sh=delta;
}

void ItemBuff::Set_AddFireResist(int delta)
{
	tp=(ItemBuffType)ItemBuff_FireResist;
	sh=delta;
}

void ItemBuff::Set_AddElecResist(int delta)
{
	tp=(ItemBuffType)ItemBuff_ElecResist;
	sh=delta;
}

void ItemBuff::Set_AddColdResist(int delta)
{
	tp=(ItemBuffType)ItemBuff_ColdResist;
	sh=delta;
}

void ItemBuff::Set_AddPoisonResist(int delta)
{
	tp=(ItemBuffType)ItemBuff_PoisonResist;
	sh=delta;
}

void ItemBuff::Set_AddPhysDmg_Bow(int delta)
{
	tp=(ItemBuffType)ItemBuff_PhysDmg_Bow;
	sh=delta;
}

void ItemBuff::Set_AddPhysDmgRate_Bow(int delta)
{
	tp=(ItemBuffType)ItemBuff_PhysDmgRate_Bow;
	sh=delta;
}



//////////////////////////////////////////////////////////////////////////
//LevelItemState
BOOL LevelItemState::AddItemBuff(ItemBuff &buff)
{
	if (nBuffs>=ARRAY_SIZE(buffs))
	{
		assert(FALSE);
		return FALSE;
	}
	buffs[nBuffs]=buff;
	nBuffs++;

	return TRUE;
}
