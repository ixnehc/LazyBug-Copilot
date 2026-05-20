
#pragma once

#include "../fastdelegate/FastDelegate.h"

#include "class/class.h"

#include "../enums/enums.h"


enum AvtrState
{
	AVS_None=0,

	AVS_NotMove=1,
	AVS_Move=2,
	AVS_MoveTo=3,

	AVS_NotTurn=4,
	AVS_Turn=5,
	AVS_TurnTo=6,

	AVS_NotSit_obsolete=7,
	AVS_Sit_obsolete=8,//坐下
	AVS_UnSit_obsolete=9,//

	AVS_NotJump=10,
	AVS_Jump=11,
	AVS_JumpLand=12,

	//KOing表示击倒(Knock Out)
	AVS_NotKO_obsolete=13,
	AVS_KO_obsolete=14,//击倒
	AVS_UnKO_obsolete=15,//击倒后爬起

	AVS_NotBattle_obsolete=16,
	AVS_Battle_obsolete=17,

	//表演
	AVS_NotAct_0=18,
	AVS_NotAct_1=19,
	AVS_NotAct_2=20,
	AVS_NotAct_3=21,
	AVS_Act_0=22,
	AVS_Act_1=23,
	AVS_Act_2=24,
	AVS_Act_3=25,

	AVS_NotFly=26,
	AVS_FlyUp=27,
	AVS_Flying=28,
	AVS_FlyDescend=29,
	AVS_FlyDown=30,

	AVS_NotMount=31,
	AVS_Mounting=32,

	AVS_NotImpel=33,
	AVS_Impel=34,

	AVS_MoveTeleport=35,

	AVS_NotReside=36,
	AVS_Reside=37,

	//注意,添加新的state时,不要修改原来state的值

	AvatarState_Max=64,//最多可以有64个状态

};

BEGIN_ENUMS(AvtrState,AVS_)
	ENUM_ENTRY(AVS_None)

	ENUM_ENTRY_D(AVS_NotMove,"没有移动")
	ENUM_ENTRY_D(AVS_Move,"移动中")
	ENUM_ENTRY_D(AVS_MoveTo,"移向目的中")

	ENUM_ENTRY_D(AVS_NotTurn,"没有旋转")
	ENUM_ENTRY_D(AVS_Turn,"旋转中")
	ENUM_ENTRY_D(AVS_TurnTo,"旋转向指定角度中")

	ENUM_ENTRY_D(AVS_NotJump,"没有跳跃")
	ENUM_ENTRY_D(AVS_Jump,"跳跃中")
	ENUM_ENTRY_D(AVS_JumpLand,"跳跃着陆")

	ENUM_ENTRY_D(AVS_NotAct_0,"没有Act")
	ENUM_ENTRY_D(AVS_Act_0,"正在执行一个Act")

	ENUM_ENTRY_D(AVS_NotAct_1,"")
	ENUM_ENTRY_D(AVS_Act_1,"")

	ENUM_ENTRY_D(AVS_NotAct_2,"")
	ENUM_ENTRY_D(AVS_Act_2,"")

	ENUM_ENTRY_D(AVS_NotAct_3,"")
	ENUM_ENTRY_D(AVS_Act_3,"")

	ENUM_ENTRY_D(AVS_NotFly,"没有飞行")
	ENUM_ENTRY_D(AVS_FlyUp,"起飞中")
	ENUM_ENTRY_D(AVS_Flying,"飞行中")
	ENUM_ENTRY_D(AVS_FlyDescend,"飞行着陆前的降落")
	ENUM_ENTRY_D(AVS_FlyDown,"飞行着陆")

	ENUM_ENTRY_D(AVS_NotMount,"")
	ENUM_ENTRY_D(AVS_Mounting,"")

	ENUM_ENTRY_D(AVS_NotImpel,"")
	ENUM_ENTRY_D(AVS_Impel,"")

	ENUM_ENTRY_D(AVS_NotReside,"没有驻留")
	ENUM_ENTRY_D(AVS_Reside,"驻留")

END_ENUMS();




#define CHAR_ACT_CHANNEL_MAX 4




struct AvtrStateMask
{
	AvtrStateMask()
	{
		v=0;
	}
	void Reset()
	{
		v=0;
	}
	BOOL Test(AvtrState s1)
	{
		if (v& 
			(((unsigned __int64)1)<<s1)
			)
			return TRUE;
		return FALSE;
	}
	BOOL Test(AvtrState s1,AvtrState s2)
	{
		if (v& 
			((((unsigned __int64)1)<<s1)
			|(((unsigned __int64)1)<<s2)
			))
			return TRUE;
		return FALSE;
	}
	BOOL Test(AvtrState s1,AvtrState s2,AvtrState s3)
	{
		if (v& 
			((((unsigned __int64)1)<<s1)
			|(((unsigned __int64)1)<<s2)
			|(((unsigned __int64)1)<<s3)
			))
			return TRUE;
		return FALSE;
	}
	BOOL Test(AvtrState s1,AvtrState s2,AvtrState s3,AvtrState s4)
	{
		if (v& 
			((((unsigned __int64)1)<<s1)
			|(((unsigned __int64)1)<<s2)
			|(((unsigned __int64)1)<<s3)
			|(((unsigned __int64)1)<<s4)
			))
			return TRUE;
		return FALSE;
	}
	BOOL Test(AvtrState s1,AvtrState s2,AvtrState s3,AvtrState s4,AvtrState s5)
	{
		if (v& 
			((((unsigned __int64)1)<<s1)
			|(((unsigned __int64)1)<<s2)
			|(((unsigned __int64)1)<<s3)
			|(((unsigned __int64)1)<<s4)
			|(((unsigned __int64)1)<<s5)
			))
			return TRUE;
		return FALSE;
	}
	void Set(AvtrState s)
	{
		v=(((unsigned __int64)1)<<s);
	}
	void Add(AvtrState s)
	{
		v|=(((unsigned __int64)1)<<s);
	}
	void Remove(AvtrState s)
	{
		v&=~(((unsigned __int64)1)<<s);

	}
	void Remove(AvtrState s1,AvtrState s2)
	{
		v&=~
			((((unsigned __int64)1)<<s1)
			|(((unsigned __int64)1)<<s2));
	}
	void Remove(AvtrState s1,AvtrState s2,AvtrState s3)
	{
		v&=~
			((((unsigned __int64)1)<<s1)
			|(((unsigned __int64)1)<<s2)
			|(((unsigned __int64)1)<<s3));
	}
	void Remove(AvtrState s1,AvtrState s2,AvtrState s3,AvtrState s4)
	{
		v&=~
			((((unsigned __int64)1)<<s1)
			|(((unsigned __int64)1)<<s2)
			|(((unsigned __int64)1)<<s3)
			|(((unsigned __int64)1)<<s4));
	}

	void Add(AvtrStateMask m)
	{
		v|=m.v;
	}
	void Remove(AvtrStateMask m)
	{
		v&=~m.v;
	}
	unsigned __int64 v;
};

struct AvtrCmdFlags
{
	void Zero()
	{
		flags=0;
	}
	union 
	{
		struct 
		{
			DWORD bMove:1;
			DWORD bMoveTo:1;
			DWORD bTurn:1;
			DWORD bTurnTo:1;
		};
		DWORD flags;
	};
};

enum AvtrMoveStage
{
	AvtrMoveStage_None=0,
	AvtrMoveStage_NotMove,
	AvtrMoveStage_RotateOnSpot,
	AvtrMoveStage_Faced,
	AvtrMoveStage_Reached,//到达目标停止
	AvtrMoveStage_Abort,//未到达目标停止
	AvtrMoveStage_StartFw,
	AvtrMoveStage_StartRot,
	AvtrMoveStage_Move,
	AvtrMoveStage_Stop,

	//XXXXX:More Move Stage
};

enum AvtrFootStep
{
	AvtrFootStep_None,
	AvtrFootStep_LeftPass,
	AvtrFootStep_Left,
	AvtrFootStep_RightPass,
	AvtrFootStep_Right,

	AvtrFootStep_ForceDword=0xffffffff,
};
#define GSemConstraint_AvtrFootStep "LeftPass:1,Left:2,RightPass:3,Right:4"