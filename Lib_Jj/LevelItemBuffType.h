#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "enums/enums.h"


enum EItemBuffType
{
	ItemBuff_None=0,
	ItemBuff_AddMaxHP,//加最大HP
	ItemBuff_RecoverHP,//HP恢复
	ItemBuff_AddFullSP,//加FullSP
	ItemBuff_RecoverSP,//SP恢复
	ItemBuff_FireResist,//火抗
	ItemBuff_ElecResist,//电抗
	ItemBuff_ColdResist,//冰抗
	ItemBuff_PoisonResist,//毒抗
	ItemBuff_HolyResist_Obsolete,//神圣抗
	ItemBuff_EvilResist_Obsolete,//恶魔抗
	ItemBuff_MoveSpeed,//移动速度
	ItemBuff_BlockRate_Obsolete,//格挡率
	ItemBuff_AttackSpeed,//攻击速度
	ItemBuff_Strength_Obsolete,
	ItemBuff_Dexterity_Obsolete,
	ItemBuff_Magic_Obsolete,
	ItemBuff_Leadership_Obsolete,//统御
	ItemBuff_PhysDmg,//物理伤害
	ItemBuff_PhysDmgRate,//物理伤害百分比
	ItemBuff_FireDmg,//火伤
	ItemBuff_ElecDmg,//电伤
	ItemBuff_ColdDmg,//冰伤
	ItemBuff_PoisonDmg,//毒伤
	ItemBuff_ElementDmgRate,//元素伤害百分比
	ItemBuff_PhysDef,//物理防御
	ItemBuff_PhysDefRate,//物理防御百分比
	ItemBuff_AttackSpeed_Bow,//弓的攻击速度
	ItemBuff_PhysDef_Base,//基础物理防御
	ItemBuff_PhysDmg_Bow,//弓的物理伤害
	ItemBuff_PhysDmgRate_Bow,//弓的物理伤害百分比

	//XXXXX: more ItemBuffType

	ItemBuff_Max,

	ItemBuff_ForceDword=0xffffffff,
};


BEGIN_ENUMS(EItemBuffType,ItemBuff_)

	ENUM_ENTRY_D(ItemBuff_None,"n/a")
	ENUM_ENTRY_D(ItemBuff_AddMaxHP,"加最大HP")
	ENUM_ENTRY_D(ItemBuff_RecoverHP,"加恢复HP")
	ENUM_ENTRY_D(ItemBuff_AddFullSP,"加FullSP")
	ENUM_ENTRY_D(ItemBuff_RecoverSP,"加恢复SP")
	ENUM_ENTRY_D(ItemBuff_FireResist,"加火抗")
	ENUM_ENTRY_D(ItemBuff_ElecResist,"加电抗")
	ENUM_ENTRY_D(ItemBuff_ColdResist,"加冰抗")
	ENUM_ENTRY_D(ItemBuff_PoisonResist,"加毒抗")
	ENUM_ENTRY_D(ItemBuff_HolyResist_Obsolete,"加神圣抗(Obsolete)")
	ENUM_ENTRY_D(ItemBuff_EvilResist_Obsolete,"加恶魔抗(Obsolete)")
	ENUM_ENTRY_D(ItemBuff_MoveSpeed,"加移动速度")
	ENUM_ENTRY_D(ItemBuff_BlockRate_Obsolete,"加格挡率(Obsolete)")
	ENUM_ENTRY_D(ItemBuff_AttackSpeed,"加攻击速度")
	ENUM_ENTRY_D(ItemBuff_Strength_Obsolete,"加力量(Obsolete)")
	ENUM_ENTRY_D(ItemBuff_Dexterity_Obsolete,"加敏捷(Obsolete)")
	ENUM_ENTRY_D(ItemBuff_Magic_Obsolete,"加智慧(Obsolete)")
	ENUM_ENTRY_D(ItemBuff_Leadership_Obsolete,"加统御(Obsolete)")
	ENUM_ENTRY_D(ItemBuff_PhysDmg,"加物理伤害")
	ENUM_ENTRY_D(ItemBuff_PhysDmgRate,"加物理伤害百分比")
	ENUM_ENTRY_D(ItemBuff_FireDmg,"加火伤")
	ENUM_ENTRY_D(ItemBuff_ElecDmg,"加电伤")
	ENUM_ENTRY_D(ItemBuff_ColdDmg,"加冰伤")
	ENUM_ENTRY_D(ItemBuff_PoisonDmg,"加毒伤")
	ENUM_ENTRY_D(ItemBuff_ElementDmgRate,"加元素伤害百分比")
	ENUM_ENTRY_D(ItemBuff_PhysDef,"加物理防御")
	ENUM_ENTRY_D(ItemBuff_PhysDefRate,"加物理防御百分比")
	ENUM_ENTRY_D(ItemBuff_AttackSpeed_Bow,"加弓攻击速度")
	ENUM_ENTRY_D(ItemBuff_PhysDef_Base,"加基础物理防御")
	ENUM_ENTRY_D(ItemBuff_PhysDmg_Bow,"加弓物理伤害")
	ENUM_ENTRY_D(ItemBuff_PhysDmgRate_Bow,"加弓物理伤害百分比")

	//XXXXX: more ItemBuffType

END_ENUMS();



