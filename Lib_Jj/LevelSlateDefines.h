#pragma once

#define LevelSlate_MaxLink 16

typedef short LevelSlateIdx;
#define LevelSlateIdx_Invalid  (-1)

struct SlateGrpEntry
{
	StringID nm;
	std::vector<LevelGUID> refs;//AssetUID类型

	BEGIN_GOBJ_PURE(SlateGrpEntry,1);

		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"石板组名称"), "石板组名称" );

		GELEM_VARVECTOR(LevelGUID,refs)
			GELEM_EDITVAR("石板引用",GVT_U,GSem(GSem_Unknown,"AssetUIDSet"),"石板引用");

	END_GOBJ();

};

struct SlateSpaceDefine
{
	std::vector<LevelGUID> base;
	std::vector<LevelGUID> axisX;
	std::vector<LevelGUID> axisY;

	BEGIN_GOBJ_PURE(SlateSpaceDefine,1);

		GELEM_VARVECTOR(LevelGUID,base) GELEM_VERSION(2)
			GELEM_EDITVAR("原点",GVT_U,GSem(GSem_Unknown,"AssetUIDSet"),"原点石板");
		GELEM_VARVECTOR(LevelGUID,axisX) GELEM_VERSION(2)
			GELEM_EDITVAR("X轴",GVT_U,GSem(GSem_Unknown,"AssetUIDSet"),"X轴石板");
		GELEM_VARVECTOR(LevelGUID,axisY) GELEM_VERSION(2)
			GELEM_EDITVAR("Y轴",GVT_U,GSem(GSem_Unknown,"AssetUIDSet"),"Y轴石板");

	END_GOBJ();

};


enum LevelSlateFamily
{
	LevelSlateFamily_A=0,
	LevelSlateFamily_B,
};


enum LevelSlateType
{
	LevelSlateType_None=0,
	LevelSlateTypeA_Blank,
	LevelSlateTypeA_Teeth,
	LevelSlateTypeA_Torch,
	LevelSlateTypeA_Cure,
	LevelSlateTypeA_Door_Entry,
	LevelSlateTypeA_Door_Exit,
	LevelSlateTypeA_Death,
	LevelSlateTypeA_Life,
	LevelSlateTypeA_Key,
	LevelSlateTypeA_Lock,
	LevelSlateTypeA_Merchant_Obsolete,
	LevelSlateTypeA_Fence,
	LevelSlateTypeA_FenceSwitch,
	LevelSlateTypeA_Trunk_Obsolete,
	LevelSlateTypeA_Final,
	LevelSlateTypeA_CupHP_Obsolete,
	LevelSlateTypeA_SwitchPointer,
	//XXXXX:MoreSlateType

	LevelSlateTypeB_Cross=81,
	LevelSlateTypeB_Cross_x2,
	LevelSlateTypeB_Ver,
	LevelSlateTypeB_Hor,
	LevelSlateTypeB_Full,
	LevelSlateTypeB_Ring,
	LevelSlateTypeB_Ring_x2,
	LevelSlateTypeB_Right,
	LevelSlateTypeB_Left,
	LevelSlateTypeB_Up,
	LevelSlateTypeB_Down,
	LevelSlateTypeB_Ascend,
	LevelSlateTypeB_Descend,
	LevelSlateTypeB_LeftUp,
	LevelSlateTypeB_LeftDown,
	LevelSlateTypeB_RightDown,
	LevelSlateTypeB_RightUp,
	LevelSlateTypeB_Rune01,
	LevelSlateTypeB_Rune02,
	LevelSlateTypeB_Rune03,
	LevelSlateTypeB_Rune04,
	LevelSlateTypeB_Rune05,
	LevelSlateTypeB_Rune06,
	//XXXXX:MoreSlateTypeB

	LevelSlateType_Max,

	//注意最多不要超过255个类型
	LevelStateType_ForceDword=0xffffffff,
};

enum LevelSlatesState
{
	LevelSlatesState_None,
	LevelSlatesState_NotInSlates,
	LevelSlatesState_Idle,
	LevelSlatesState_Process,
};


