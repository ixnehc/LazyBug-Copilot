#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "records/records.h"

#include "LevelDefines.h"

struct LevelItemPrice
{
	DWORD gold;
	DWORD goldVary;
	DWORD gem;
	DWORD gemVary;

	BEGIN_GOBJ_PURE(LevelItemPrice,1);

		GELEM_VAR_INIT(DWORD,gold,10);
			GELEM_EDITVAR("金子",GVT_U,GSem_Interger,"金子");
		GELEM_VAR_INIT(DWORD,goldVary,0);
			GELEM_EDITVAR("金子浮动",GVT_U,GSem_Interger,"金子浮动");

		GELEM_VAR_INIT(DWORD,gem,0);
			GELEM_EDITVAR("宝石",GVT_U,GSem_Interger,"宝石");
		GELEM_VAR_INIT(DWORD,gemVary,0);
			GELEM_EDITVAR("宝石浮动",GVT_U,GSem_Interger,"宝石浮动");

	END_GOBJ();
	
};


struct LevelRecordItem:public CRecord
{
	DEFINE_CLASS(LevelRecordItem);

	std::string Name;
	unsigned __int64 idEquip;
	unsigned __int64 idDiscard;
	unsigned __int64 idAltar;
	RecordID clss;
	RecordID skill;//技能
	std::string Icon;
	std::string IconConsumable;
	std::string IconDisabled;
	std::string IconToggleOn;
	std::string EquipIcon;
	std::string InvIcon;
	std::string InvIconBg;
	std::string InvIconConsumed;

	StringID desc;//功能描述
	LevelArtifactType tpArtifact;
	LevelAbilityType tpAbility;
	BOOL bAllowStack;

	LevelGrade grdTC;

	LevelItemPrice priceVendor;

	int DmgLo;
	int DmgHi;
	int Def;

	std::vector<RecordID> seeds;

	BEGIN_GOBJ_PURE(LevelRecordItem,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"道具的名称");
		GELEM_VAR_INIT( StringID,desc,StringID_Invalid);	
			GELEM_EDITVAR( "道具描述", GVT_U, GSem(GSem_StringID,"道具描述"), "道具的描述文字" );
		GELEM_VAR_INIT(unsigned __int64,idEquip,0);
			GELEM_EDITVAR("装备时Proto",GVT_Bx8,GSem_ProtoPath,"道具装备在角色身上的Proto");
		GELEM_VAR_INIT(unsigned __int64,idDiscard,0);
			GELEM_EDITVAR("掉落时Proto",GVT_Bx8,GSem_ProtoPath,"道具掉落在地上时的Proto");
		GELEM_VAR_INIT(unsigned __int64,idAltar,0);
			GELEM_EDITVAR("祭坛上的Proto",GVT_Bx8,GSem_ProtoPath,"道具在祭坛上时的Proto");
		GELEM_VAR_INIT(RecordID,clss,RecordID_Invalid);
			GELEM_EDITVAR("道具类型",GVT_U,GSem(GSem_RecordID,"itemclasses"),"该道具属于什么类型");
		GELEM_VAR_INIT(LevelArtifactType,tpArtifact,LevelArtifact_None);
			GELEM_EDITVAR("神器类型",GVT_S,GSem(GSem_Interger,LevelArtifactConstraintStr),"是哪一种神器");
		GELEM_VAR_INIT(LevelAbilityType,tpAbility,LevelAbilityType_None);
			GELEM_EDITVAR("术能类型",GVT_S,GSem(GSem_Interger,LevelAbilityConstraintStr),"有哪一种术能");
		GELEM_VAR_INIT(BOOL,bAllowStack,TRUE);
			GELEM_EDITVAR("是否允许堆叠",GVT_S,GSem_Boolean,"是否允许堆叠");
		GELEM_VAR_INIT(RecordID,skill,RecordID_Invalid);
			GELEM_EDITVAR("道具技能",GVT_U,GSem(GSem_RecordID,"skills"),"依附在道具上的技能");
		GELEM_STRING_INIT(Icon,"");
			GELEM_EDITVAR("图标",GVT_String,GSem_TexturePartPath,"道具图标的贴图");
		GELEM_STRING_INIT(IconConsumable,"");
			GELEM_EDITVAR("图标(消耗品)",GVT_String,GSem_TexturePartPath,"道具图标的贴图(消耗品)");
		GELEM_STRING_INIT(IconToggleOn,"");
			GELEM_EDITVAR("图标(激活状态)",GVT_String,GSem_TexturePartPath,"道具图标的贴图(激活状态)");
		GELEM_STRING_INIT(IconDisabled,"");
			GELEM_EDITVAR("图标(禁用状态)",GVT_String,GSem_TexturePartPath,"道具图标的贴图(禁用状态)");
		GELEM_STRING_INIT(EquipIcon,"");
			GELEM_EDITVAR("装备图标",GVT_String,GSem_TexturePartPath,"道具图标的贴图(装备栏)");
		GELEM_STRING_INIT(InvIcon,"");
			GELEM_EDITVAR("背包图标",GVT_String,GSem_TexturePartPath,"道具图标的贴图(背包)");
		GELEM_STRING_INIT(InvIconConsumed,"");
			GELEM_EDITVAR("背包图标(已消耗)",GVT_String,GSem_TexturePartPath,"道具图标的贴图(背包内,已消耗)");
		GELEM_STRING_INIT(InvIconBg,"");
			GELEM_EDITVAR("背包图标背景",GVT_String,GSem_TexturePartPath,"道具图标的背景贴图(背包)");
		GELEM_VAR_INIT(int,grdTC,0);
			GELEM_EDITVAR("TC等级",GVT_S,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"TreasureClass的等级");

		GELEM_OBJ(LevelItemPrice,priceVendor);
			GELEM_EDITOBJ("价格(Vendor)","Vendor销售价格");
		GELEM_VAR_INIT(int,DmgLo,0);
			GELEM_EDITVAR("最小伤害",GVT_S,GSem_Interger,"最小伤害");
		GELEM_VAR_INIT(int,DmgHi,0);
			GELEM_EDITVAR("最大伤害",GVT_S,GSem_Interger,"最大伤害");
		GELEM_VAR_INIT(int,Def,0);
			GELEM_EDITVAR("防御",GVT_S,GSem_Interger,"基础防御");


		GELEM_VARVECTOR_INIT(RecordID,seeds,RecordID_Invalid);
			GELEM_EDITVAR("可以出现的Buff的Seed",GVT_U,GSem(GSem_RecordID,"itemseeds"),"该类型的道具可以出现那些Buff");

	END_GOBJ();


};