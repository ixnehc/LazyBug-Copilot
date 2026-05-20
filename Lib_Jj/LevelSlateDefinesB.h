#pragma once

#include "LevelSlateDefines.h"

#define GSemConstraint_LevelSlateTypeB		\
	"n/a:0"						\
	",Cross:81"						\
	",Cross_x2:82"						\
	",Vertical:83"						\
	",Horizontal:84"						\
	",Full:85"				\
	",Ring:86"				\
	",Ring_x2:87"						\
	",Right:88"							\
	",Left:89"							\
	",Up:90"							\
	",Down:91"							\
	",Ascend:92"							\
	",Descend:93"							\
	",LeftUp:94"							\
	",LeftDown:95"							\
	",RightDown:96"							\
	",RightUp:97"							\
	",Rune01:98"							\
	",Rune02:99"							\
	",Rune03:100"							\
	",Rune04:101"							\
	",Rune05:102"							\
	",Rune06:103"
//XXXXX:MoreSlateTypeB


inline const char *LevelSlateB_NameFromType(LevelSlateType tp)
{
	switch(tp)
	{
	case LevelSlateTypeB_Cross:
		return "Cross";
	case LevelSlateTypeB_Cross_x2:
		return "Cross_x2";
	case LevelSlateTypeB_Ver:
		return "Vertical";
	case LevelSlateTypeB_Hor:
		return "Horizontal";
	case LevelSlateTypeB_Full:
		return "Full";
	case LevelSlateTypeB_Ring:
		return "Ring";
	case LevelSlateTypeB_Ring_x2:
		return "Ring_x2";
	case LevelSlateTypeB_Right:
		return "Right";
	case LevelSlateTypeB_Left:
		return "Left";
	case LevelSlateTypeB_Down:
		return "Down";
	case LevelSlateTypeB_Up:
		return "Up";
	case LevelSlateTypeB_Ascend:
		return "Ascend";
	case LevelSlateTypeB_Descend:
		return "Descend";
	case LevelSlateTypeB_LeftUp:
		return "LeftUp";
	case LevelSlateTypeB_LeftDown:
		return "LeftDown";
	case LevelSlateTypeB_RightUp:
		return "RightUp";
	case LevelSlateTypeB_RightDown:
		return "RightDown";
	case LevelSlateTypeB_Rune01:
		return "Rune01";
	case LevelSlateTypeB_Rune02:
		return "Rune02";
	case LevelSlateTypeB_Rune03:
		return "Rune03";
	case LevelSlateTypeB_Rune04:
		return "Rune04";
	case LevelSlateTypeB_Rune05:
		return "Rune05";
	case LevelSlateTypeB_Rune06:
		return "Rune06";
	}
	//XXXXX:MoreSlateTypeB
	return "n/a";
}

struct LevelSlateB_Status
{
	LevelSlateB_Status()
	{
		Zero();
	}
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL IsRevealed()	{		return nRevealed>0;	}
	BOOL IsLock()	{		return tp>=LevelSlateTypeB_Rune01&&tp<=LevelSlateTypeB_Rune06;	}
	BOOL IsPath()	{		return bPath;	}
	LevelSlateType tp;
	DWORD bEntrance:1;
	DWORD bExit:1;
	DWORD bPath:1;
	DWORD nRevealed:5;
};


struct SlatesBData
{
	SlatesBData()
	{
		w=h=0;
	}
	int w,h;
	struct Slate
	{
		BYTE tp;
		BYTE bPath;
		BYTE bLock;
	};
	std::vector<Slate> buf;
};

#define SLATESB_MAX_STAMP 9