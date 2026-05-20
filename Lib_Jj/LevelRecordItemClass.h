#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "records/records.h"

#include "LevelItemBuffType.h"

struct ItemBuffInfo
{
	EItemBuffType tp;
	float wt;

	BEGIN_GOBJ_PURE(ItemBuffInfo,1);

		GELEM_VAR_INIT(EItemBuffType,tp,ItemBuff_None);
			GELEM_EDITVAR("类型",GVT_S,GSem(GSem_Interger,Enums_GetGSemStr(EItemBuffType)),"类型");
		GELEM_VAR_INIT(float,wt,1.0f);
			GELEM_EDITVAR("权重",GVT_F,GSem(GSem_Float,"0.0001,100.0,0.0001"),"Buff出现的权重");

	END_GOBJ();


};


struct LevelRecordItemClass:public CRecord
{
	DEFINE_CLASS(LevelRecordItemClass);

	BOOL IsValidEquipPart()
	{
		return part<EquipPart_Max;
	}

	std::string Name;
	RecordID posture;
	int part;
	BYTE wSlot;//背包栏里几格宽
	BYTE hSlot;//背包栏里几格高
	LevelItemCategory category;//大类
	LevelResourceType tpRes;//资源类型

	std::vector<ItemBuffInfo> seeds;

	BEGIN_GOBJ_PURE(LevelRecordItemClass,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"名称");
		GELEM_VAR_INIT(RecordID,posture,RecordID_Invalid);
			GELEM_EDITVAR("姿势",GVT_U,GSem(GSem_RecordID,"postures"),"装备该类型道具后,角色要变成什么姿势");
		GELEM_VAR_INIT(int,part,0);
		GELEM_EDITVAR("装备部位",GVT_S,GSem(GSem_Interger,"头部装备:0,盾牌:1,武器:2,手套:3,装甲:4,鞋子:5,头:6,附件01:7,附件02:8,附件03:9,附件04:10,魔法物件:12,n/a:1000"),"该类型道具装备在哪个部位");
		GELEM_VAR_INIT(BYTE,wSlot,1);
			GELEM_EDITVAR("格宽度",GVT_B,GSem(GSem_Interger,"1:1,2:2"),"背包栏里几格宽");
		GELEM_VAR_INIT(BYTE,hSlot,1);
			GELEM_EDITVAR("格高度",GVT_B,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6"),"背包栏里几格高");

		GELEM_VAR_INIT(LevelItemCategory,category,LevelItemCategory_None);
			GELEM_EDITVAR("大类",GVT_S,GSem(GSem_Interger,"n/a,武器,防具,饰品,资源,残片,神器"),"该道具的大的类型");

		GELEM_VAR_INIT(LevelResourceType,tpRes,LevelResource_None);
			GELEM_EDITVAR("资源类型",GVT_S,GSem(GSem_Interger,LevelResourceType_SemConstraint),"该道具的资源类型");

		GELEM_OBJVECTOR(ItemBuffInfo,seeds)
			GELEM_EDITOBJ("可出现Buff","这个类型的Item上可以出现哪些Buff,它们的几率是多少");

	END_GOBJ();


};