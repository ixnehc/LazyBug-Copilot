#pragma once

#include "LevelSlateDefines.h"


#define GSemConstraint_LevelSlateTypeA		\
	"n/a:0"						\
	",空白:1"						\
	",利齿:2"						\
	",火炬:3"						\
	",治愈:4"						\
	",门(入口):5"				\
	",门(出口):6"				\
	",死亡:7"						\
	",生命:8"						\
	",钥匙:9"						\
	",锁:10"							\
	",商人索尔(Obsolete):11"				\
	",铁栏:12"						\
	",铁栏开关:13"				\
	",宝箱(Obsolete):14"						\
	",终止:15"				\
	",生命圣杯(Obsolete):16"				\
	",Switch指针:17"
//XXXXX:MoreSlateType



inline const char *LevelSlateA_NameFromType(LevelSlateType tp)
{
	switch(tp)
	{
	case LevelSlateTypeA_Blank:
		return "空白";
	case LevelSlateTypeA_Teeth:
		return "利齿";
	case LevelSlateTypeA_Torch:
		return "火炬";
	case LevelSlateTypeA_Cure:
		return "治愈";
	case LevelSlateTypeA_Door_Entry:
		return "门(入口)";
	case LevelSlateTypeA_Door_Exit:
		return "门(出口)";
	case LevelSlateTypeA_Death:
		return "死亡";
	case LevelSlateTypeA_Life:
		return "生命";
	case LevelSlateTypeA_Key:
		return "钥匙";
	case LevelSlateTypeA_Lock:
		return "锁";
	case LevelSlateTypeA_Fence:
		return "铁栏";
	case LevelSlateTypeA_FenceSwitch:
		return "铁栏开关";
	case LevelSlateTypeA_Final:
		return "终止";
	case LevelSlateTypeA_SwitchPointer:
		return "Switch指针";
		//XXXXX:MoreSlateType
	}

	return "n/a";
}



enum LevelSlateA_Cover
{
	LevelSlateA_Cover_None=0,
	LevelSlateA_Cover_Normal,
	LevelSlateA_Cover_Silver,
	LevelSlateA_Cover_Gold,
	LevelSlateA_Cover_Max,

	LevelStateCover_ForceDword=0xffffffff,
};

#define GSemConstraint_LevelSlateA_Cover		\
	"n/a"						\
	",普通"						\
	",银色"						\
	",金色"
//XXXXX:MoreSlateCover


inline BOOL SlateA_IsKnownState(LevelSlateType tp)
{
	switch(tp)
	{
		case LevelSlateTypeA_Torch:
			return TRUE;
		case LevelSlateTypeA_Fence:
			return TRUE;
	}
	//XXXXX:MoreSlateType
	return FALSE;
}

inline BOOL SlateA_IsEmbeddingSlate(LevelSlateType tp)
{
	switch(tp)
	{
		case LevelSlateTypeA_Final:
			return TRUE;
	}
	//XXXXX:MoreSlateType
	return FALSE;
}

enum LevelSlateA_SwitchChannel
{
	LevelSlateA_SwitchChannel_None,
	LevelSlateA_SwitchChannel_A,
	LevelSlateA_SwitchChannel_B,
	LevelSlateA_SwitchChannel_C,
	LevelSlateA_SwitchChannel_D,

	LevelSlateA_SwitchChannel_Max,

	LevelSlateA_SwitchChannel_ForceDword=0xffffffff,
};

#define GSemConstraint_LevelSlateA_SwitchChannel		\
"n/a"						\
",A"						\
",B"						\
",C"						\
",D"

inline const char *SlateA_NameFromSwitchChannel(LevelSlateA_SwitchChannel channel)
{
	switch(channel)
	{
		case LevelSlateA_SwitchChannel_A:
			return "A";
		case LevelSlateA_SwitchChannel_B:
			return "B";
		case LevelSlateA_SwitchChannel_C:
			return "C";
		case LevelSlateA_SwitchChannel_D:
			return "D";
	}

	return "n/a";
}

#define LevelSlateA_MaxSwitchLocks 10

enum LevelSlateA_ButtonChannel
{
	LevelSlateA_ButtonChannel_None,
	LevelSlateA_ButtonChannel_A,
	LevelSlateA_ButtonChannel_B,
	LevelSlateA_ButtonChannel_C,
	LevelSlateA_ButtonChannel_D,

	LevelSlateA_ButtonChannel_Max,

	LevelSlateA_ButtonChannel_ForceDword=0xffffffff,
};

#define GSemConstraint_LevelSlateA_ButtonChannel		\
	"n/a"						\
	",A"						\
	",B"						\
	",C"						\
	",D"

inline const char *SlateA_NameFromButtonChannel(LevelSlateA_ButtonChannel channel)
{
	switch(channel)
	{
	case LevelSlateA_ButtonChannel_A:
		return "A";
	case LevelSlateA_ButtonChannel_B:
		return "B";
	case LevelSlateA_ButtonChannel_C:
		return "C";
	case LevelSlateA_ButtonChannel_D:
		return "D";
	}

	return "n/a";
}

#define LevelSlateA_MaxButtonLocks 10
typedef WORD SlateA_ButtonLockMask;//Bit位数与LevelSlateA_MaxButtonLocks要匹配

#define LevelSlateA_MaxButtonChips 30



typedef WORD SlateA_EdgeLockMask;//Bit位数与LevelSlate_MaxLink要匹配
struct LevelSlateA_CoverStatus
{
	LevelSlateA_CoverStatus()
	{
		Zero();
	}
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL IsSwitchLock()
	{
		return nSwitches>0?TRUE:FALSE;
	}
	BOOL CheckSwitchLocked()
	{
		if ((nSwitches>0)&&(nUnlockedSwitches<nSwitches))
			return TRUE;
		return FALSE;
	}
	BOOL IsButtonLock()
	{
		if (channelButton!=LevelSlateA_ButtonChannel_None)
			return nButtons>0?TRUE:FALSE;
		return FALSE;
	}
	BOOL CheckButtonLocked()
	{
		if (channelButton!=LevelSlateA_ButtonChannel_None)
		{
			if ((nButtons>0)&&(locksButton!=0))
				return TRUE;
		}
		return FALSE;
	}

	BOOL CheckEdgeLocked()
	{
		if (nEdgeLocks>0)
		{
			if (locksEdge!=0)
				return TRUE;
		}
		return FALSE;
	}
	BOOL CheckLocked()
	{
		return CheckEdgeLocked()||CheckSwitchLocked()||CheckButtonLocked();
	}
	LevelSlateA_SwitchChannel channelSwitch:4;//开关的Channel
	DWORD nSwitches:4;//一共有几个开关
	DWORD nUnlockedSwitches:4;//所有开关中解锁了几个

	LevelSlateA_ButtonChannel channelButton:4;//Button的Channel
	DWORD nButtons:4;//一共有几个Button
	SlateA_ButtonLockMask locksButton;//各个Button的Lock状态

	DWORD nEdgeLocks:4;
	SlateA_EdgeLockMask locksEdge;
}; 

struct LevelSlateA_Status
{
	LevelSlateA_Status()
	{
		Zero();
	}
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL IsSwitch()
	{
		if (channelSwitch!=LevelSlateA_SwitchChannel_None)
		{
			if (!IsSwitchPointer())
				return TRUE;
		}
		return FALSE;
	}
	BOOL IsSwitchPointer()
	{
		if (tp!=LevelSlateTypeA_SwitchPointer)
			return FALSE;
		return TRUE;
	}
	BOOL IsButton()
	{
		return channelButton!=LevelSlateA_ButtonChannel_None;
	}
	LevelSlateType tp;
	DWORD bEntrance:1;
	DWORD bExit:1;
	DWORD bFlipped:1;
	DWORD bRevealed:1;
	DWORD bProcessed:1;
	DWORD bOpened:1;//For Fence
	DWORD matchkey:4;

	DWORD nGems:3;

	LevelSlateA_SwitchChannel channelSwitch:4;
	DWORD serialSwitch:4;//开关顺序号
	DWORD bSwitchActivated:1;
	DWORD radoffSwitchPointer:3;//开关Pointer的方向(8个方向)

	LevelSlateA_ButtonChannel channelButton:4;
	DWORD serialButton:4;//Button顺序号
	DWORD countButtonChips:6;


};


struct LevelSlateA_TeleportLink
{
	LevelSlateA_TeleportLink()
	{
		idxFrom=idxTo=LevelSlateIdx_Invalid;
	}
	LevelSlateIdx idxFrom;
	LevelSlateIdx idxTo;
};