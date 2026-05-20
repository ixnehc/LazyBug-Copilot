#pragma once

#include "class/class.h"



//Retinue
enum RetinueType
{
	Retinue_None,
	Retinue_Unit,
};
typedef DWORD RetinueUID;
#define RetinueUID_Invalid (0)

enum LevelRtnuCmdType
{
	LevelRtnuCmd_None=0,
	LevelRtnuCmd_CastSkill,
	LevelRtnuCmd_InvokeAgent,
	LevelRtnuCmd_ForceDword=0xffffffff,
};

#define LevelRtnuCmdType_SemConstraint "n/a,释放特定技能,InvokeAgent"

#define LEVEL_MAX_RTNUTYPE (32)//一个玩家最多可以拥有的随从的类型个数,注意是类型的个数

enum LevelRtnuRank
{
	LevelRtnuRank_None=0,
	LevelRtnuRank_Knight,
	LevelRtnuRank_Archer,
	LevelRtnuRank_Monk,
	LevelRtnuRank_Minion,

	LevelRtnuRank_Max,

	LevelRtnuRank_ForceDword=0xffffffff
};
#define LevelRtnuRank_SemConstraint "n/a,Knight,Archer,Monk,Minion"
inline const char *GetRtnuRankName(LevelRtnuRank rank)
{
	switch(rank)
	{
	case LevelRtnuRank_Knight:
		return "Knight";
	case LevelRtnuRank_Archer:
		return "Archer";
	case LevelRtnuRank_Monk:
		return "Monk";
	case LevelRtnuRank_Minion:
		return "Minion";
	}
	return "n/a";
}

enum LevelRtnuBehavior
{
	LevelRtnuBehavior_None,
	LevelRtnuBehavior_FreeAttack,
	LevelRtnuBehavior_GuardAttack,
	LevelRtnuBehavior_Pursue,
	LevelRtnuBehavior_Accompany,

	LevelRtnuBehavior_ForceDword=0xffffffff
};
#define LevelRtnuBehavior_SemConstraint "自由攻击:1,守卫攻击:2,追赶主人:3,伴随主人:4"
inline const char *GetRtnuBehaviorName(LevelRtnuBehavior bhv)
{
	switch(bhv)
	{
		case LevelRtnuBehavior_FreeAttack:
			return "自由攻击";
		case LevelRtnuBehavior_GuardAttack:
			return "守卫攻击";
		case LevelRtnuBehavior_Pursue:
			return "追赶主人";
		case LevelRtnuBehavior_Accompany:
			return "伴随主人";
	}
	return "n/a";
}



struct LevelRtnuCmd
{
	enum CmdRtnuType
	{
		CmdRtnu_None,
		CmdRtnu_All,//所有Retinue
		CmdRtnu_Rank,//具有某个Rank的所有Retinue
		CmdRtnu_ID,//发给单一的Retinue
	};

	//命令类型
	LevelRtnuCmdType tp:4;
	CmdRtnuType tpRtnu:4;

	//命令参数
	LevelObjID idTarget;
	LevelPos posTarget;

	RecordSimpleID idSkill;//tp为LevelRtnuCmd_CastSkill时有效

	//命令发给哪些Retinue
	union
	{
		LevelRtnuRank rank;//tpTarget为LevelRtnuRank时有效
		LevelObjID id;//tpTarget为CmdRtnu_ID时有效
	};

};

//一些客户端发给Server端的信息,用来帮助Rtnu进行AI控制
struct LevelRtnuHint
{
	LevelRtnuHint()
	{
		Zero();
	}
	void Zero()
	{
		radEnemy=-1.0f;
	}
	BOOL IsValid()
	{
		return (radEnemy>=0.0f);
	}
	BOOL Equals(LevelRtnuHint &other)
	{
		if (radEnemy!=other.radEnemy)
			return FALSE;
		return TRUE;
	}
	float radEnemy;//敌人最多的方向
};
