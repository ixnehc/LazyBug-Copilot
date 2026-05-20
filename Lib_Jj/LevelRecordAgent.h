#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "records/records.h"

#include "anim/animdefines.h"

#include "MagicBoardDefines.h"

#include "LevelAttrs.h"
#include "LevelObjMiscFlags.h"


struct LevelDialogInfo
{
	StringID Name;
	unsigned __int64 proto;

	BEGIN_GOBJ_PURE_UID(LevelDialogInfo,1);
		GELEM_VAR_INIT( StringID,Name,StringID_Invalid);	
			GELEM_EDITVAR( "对话框名称", GVT_U, GSem(GSem_StringID,"对话框名称"), "对话框名称" );
		GELEM_VAR_INIT(unsigned __int64,proto,0);
			GELEM_EDITVAR("对话框Proto",GVT_Bx8,GSem_ProtoPath,"对话框的Proto路径");
	END_GOBJ();
};

enum AgentResidableMode
{
	AgentResidable_None,
	AgentResidable_SingleSeat,//单个驻留位置
	AgentResidable_InfiniteSeat,//无限个驻留位置
};

enum AgentBehaviorMode
{
	AgentBehavior_None=0,
	AgentBehavior_Single,//一个统一的Behavior
	AgentBehavior_EachPlayer,//每个Player有一个Behavior
};




struct LevelRecordAgent:public CRecord
{
	DEFINE_CLASS(LevelRecordAgent);

	std::string Name;

	unsigned __int64 Src;
	BOOL bServerOnly;

	int HP;
	int SP;

	BOOL bAttackable;
	BOOL bAllowComboAttack;
	int Dmg;
	int Def;

	int Accu;
	int Evade;
	int Stun;
	int StunResist;
	int CanStun;

	EvadeEx evade;
	ResistsEx resists;

	LoAgentMiscFlags flagsMisc;

	float Radius;
	float InvokeRange;
	float Height;
	DWORD layorCollide;

	BOOL bMiniIcon;
	float scaleIconMin;

	RecordID buffDead;//死亡的Buff

	BYTE bResetCheckDay;
	BYTE bSkillDriver;
	BYTE bBuffs;
	LevelTalkMode modeTalk;
	AgentResidableMode modeResidable;
	AgentBehaviorMode modeBehavior;

	StringID idBG;
	BOOL bPersist;

	unsigned __int64 effectsStrike[16];

	DWORD resMB[MBRes_Max];

	LevelServiceType tpInvokeService;

	std::vector<LevelDialogInfo> dlgs;

	StringID nmAgentSlot;


	BEGIN_GOBJ_PURE(LevelRecordAgent,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"单位的名称");
		GELEM_VAR_INIT(BOOL,bServerOnly,FALSE);
			GELEM_EDITVAR("仅在Server端创建",GVT_S,GSem_Boolean,"是否只在Server端创建(不需要同步到Client端");
		GELEM_VAR_INIT(unsigned __int64,Src,0);
			GELEM_EDITVAR("Src Proto",GVT_Bx8,GSem_ProtoPath,"AgentSource的Proto路径,能被动态创建的Agent才需要");


		GELEM_VAR_INIT(BOOL,bAttackable,0);
			GELEM_EDITVAR("是否可被攻击",GVT_S,GSem_Boolean,"这个Agent是否可成为攻击对象");
		GELEM_VAR_INIT(BOOL,bAllowComboAttack,1);
			GELEM_EDITVAR("是否可被Combo攻击",GVT_S,GSem_Boolean,"这个Agent是否可被Combo攻击");

		GELEM_VAR_INIT(BOOL,bMiniIcon,FALSE);
			GELEM_EDITVAR("是否显示小地图图标",GVT_S,GSem_Boolean,"是否显示小地图图标");
		GELEM_VAR_INIT(float,scaleIconMin,0.0f);
			GELEM_EDITVAR("图标最小缩放",GVT_F,GSem(GSem_Float,"0.0,1.0,0.05"),"图标最小缩放");

		GELEM_VAR_INIT(int,HP,100);
			GELEM_EDITVAR("最大生命值",GVT_S,GSem_Interger,"单位的最大生命值");
		GELEM_VAR_INIT(int,SP,1000);
			GELEM_EDITVAR("最大灵气值",GVT_S,GSem_Interger,"单位的最大灵气值");

		GELEM_OBJ(EvadeEx,evade);
			GELEM_EDITOBJ("闪避","闪避");
		GELEM_OBJ(ResistsEx,resists);
			GELEM_EDITOBJ("抵抗属性","抵抗属性");

		GELEM_OBJ(LoAgentMiscFlags,flagsMisc);
			GELEM_EDITOBJ("杂七杂八的标志","杂七杂八的标志");


		GELEM_VAR_INIT(float,Radius,0.5f);
			GELEM_EDITVAR("半径",GVT_F,GSem(GSem_Float,"0,100,0"),"单位半径");
		GELEM_VAR_INIT(float,Height,1.5f);
			GELEM_EDITVAR("身高",GVT_F,GSem(GSem_Float,"0,100,0"),"单位的身高");
		GELEM_VAR_INIT(DWORD,layorCollide,3);//底层/中层
			GELEM_EDITVAR("碰撞高度层",GVT_U,GSem(GSem_Flags,"底层:1,中层:2,高层:4"),"在哪些高度层上有碰撞 ");

		GELEM_VAR_INIT(float,InvokeRange,5.0f);
			GELEM_EDITVAR("触发半径",GVT_F,GSem(GSem_Float,"0,100,0"),"触发半径");

		GELEM_VAR_INIT(RecordID,buffDead,RecordID_Invalid);
			GELEM_EDITVAR("死亡Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位死亡时的Buff");

		GELEM_VARARRAY_INIT(unsigned __int64,effectsStrike,0); 
			GELEM_EDITVAR("受击效果",GVT_Bx8,GSem(GSem_ProtoPath,
				"$Lable{穿刺,重击,火,电,冰,毒,爆炸,砸碎}"),"受到各种击打的表现效果"); GELEM_VERSION(4)//XXXXX: More DamageAttrType

		GELEM_VAR_INIT(AgentBehaviorMode,modeBehavior,AgentBehavior_EachPlayer);
			GELEM_EDITVAR("行为模式",GVT_S,GSem(GSem_Interger,"n/a:0,单个行为:1,针对每个玩家有一个行为:2"),"行为模式");
		GELEM_VAR_INIT( StringID,idBG,StringID_Invalid);	
			GELEM_EDITVAR( "行为图", GVT_U, GSem(GSem_StringID,"行为图名称"), "行为图" );
		GELEM_VAR_INIT(BOOL,bPersist,0);
			GELEM_EDITVAR("状态是否需要保存在数据库中",GVT_S,GSem_Boolean,"状态是否需要保存在数据库中");
		GELEM_VAR_INIT(BYTE,bSkillDriver,0);GELEM_VERSION(2)
			GELEM_EDITVAR("可以施放技能",GVT_B,GSem_Boolean,"是否可以施放技能");
		GELEM_VAR_INIT(BYTE,bBuffs,0);
			GELEM_EDITVAR("支持Buff",GVT_B,GSem_Boolean,"是否在这个Agent上添加Buff");
		GELEM_VAR_INIT(BYTE,bResetCheckDay,0);GELEM_VERSION(2)
			GELEM_EDITVAR("是否每日重置",GVT_B,GSem_Boolean,"这个Agent是否要每日重置");

		GELEM_VAR_INIT(LevelTalkMode,modeTalk,TalkMode_Exclusive);
			GELEM_EDITVAR("对话模式",GVT_S,GSem(GSem_Interger,"不支持对话,同时只能与一个玩家对话,同时可与多个玩家对话"),"对话模式");
		GELEM_VAR_INIT(AgentResidableMode,modeResidable,AgentResidable_None);
			GELEM_EDITVAR("可驻留模式",GVT_S,GSem(GSem_Interger,"不可驻留,只可驻留一个,可驻留无数个"),"可驻留模式");

		GELEM_VARARRAY_INIT(DWORD,resMB,0); 
			GELEM_EDITVAR("MagicBoard资源数量",GVT_S,GSem(GSem_Interger,"$Lable{//n/a,法力,金币,水晶}"),"这个Agent有多少MagicBoard资源");

		GELEM_OBJVECTOR(LevelDialogInfo,dlgs)
			GELEM_EDITOBJ("对话框","对话框信息");

		GELEM_VAR_INIT(LevelServiceType,tpInvokeService,StringID_Invalid);
			GELEM_EDITVAR("可提供的Invoke服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"可提供的Invoke服务类型");

		GELEM_VAR_INIT( StringID,nmAgentSlot,StringID_Invalid);	
			GELEM_EDITVAR( "AgentSlot名称", GVT_U, GSem(GSem_StringID,"Dummy名称"), "AgentSlot名称" );

	END_GOBJ();

	LevelAttr_Resists *GetResists()
	{
		return resists.Get();
	}
	LevelAttr_Evade *GetEvade()
	{
		return evade.Get();
	}

};
