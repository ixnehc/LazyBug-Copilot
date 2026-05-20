#pragma once



enum LevelDetectTargetFlag
{
	LevelDetectTarget_None,
	LevelDetectTarget_Enemy=1,
	LevelDetectTarget_Native=2,
	LevelDetectTarget_Ally=4,
	LevelDetectTarget_Neutral=8,
	LevelDetectTarget_Unit=16,
	LevelDetectTarget_Player=32,
	LevelDetectTarget_Agent=64,
	LevelDetectTarget_Item=128,
	LevelDetectTarget_Ground=256,
	LevelDetectTarget_Resided=512,
	LevelDetectTarget_Flying=1024,
	LevelDetectTarget_Float=2048,
};

#define LevelDetectTargetFlag_Default ((LevelDetectTargetFlag)(LevelDetectTarget_Enemy|LevelDetectTarget_Unit|LevelDetectTarget_Player|LevelDetectTarget_Ground))
#define LevelDetectTargetFlag_Relation ((LevelDetectTargetFlag)(LevelDetectTarget_Enemy|LevelDetectTarget_Native|LevelDetectTarget_Ally|LevelDetectTarget_Neutral))
#define LevelDetectTargetFlag_Type (LevelDetectTargetFlag)(LevelDetectTarget_Unit|LevelDetectTarget_Player|LevelDetectTarget_Agent|LevelDetectTarget_Item)
#define LevelDetectTargetFlag_Method (LevelDetectTargetFlag)(LevelDetectTarget_Ground|LevelDetectTarget_Resided|LevelDetectTarget_Flying|LevelDetectTarget_Float)



inline const char *LevelDetectTargetFlag_GetName(LevelDetectTargetFlag flag)
{
	switch(flag)
	{
	case LevelDetectTarget_Enemy: return "敌方";
	case LevelDetectTarget_Native:	return "本方";
	case LevelDetectTarget_Ally:	return "友方";
	case LevelDetectTarget_Neutral: return "中立";
	case LevelDetectTarget_Unit: return "单位";
	case LevelDetectTarget_Player: return "玩家";
	case LevelDetectTarget_Agent: return "Agent";
	case LevelDetectTarget_Item: return "道具";
	case LevelDetectTarget_Ground: return "地行";
	case LevelDetectTarget_Resided: return "驻留";
	case LevelDetectTarget_Flying: return "飞行";
	case LevelDetectTarget_Float: return "漂浮";
	}
	return "";
}

inline const char *LevelDetectTargetFlag_GetSemStr()
{
	return "敌方:1,本方:2,友方:4,中立:8,单位:16,玩家:32,Agent:64,道具:128,地行:256,驻留:512,飞行:1024,漂浮:2048";
}

inline const char *LevelDetectTargetFlag_GetMoveMethodSemStr()
{
	return "地行:256,驻留:512,飞行:1024,漂浮:2048";
}


inline const char *LevelDetectTargetFlags_GetRelationName(LevelDetectTargetFlag flags)
{
	static std::string nmRelation;
	LevelDetectTargetFlag flagsRelation=(LevelDetectTargetFlag)(flags&LevelDetectTargetFlag_Relation);
	nmRelation="";
	if (TRUE)
	{
		if (flagsRelation==LevelDetectTargetFlag_Relation)
			nmRelation="所有";
		else
		{
			LevelDetectTargetFlag buf[]={LevelDetectTarget_Enemy,LevelDetectTarget_Native,LevelDetectTarget_Ally,LevelDetectTarget_Neutral};
			for (int i=0;i<ARRAY_SIZE(buf);i++)
			{
				if (flagsRelation&buf[i])
				{
					if (!nmRelation.empty())
						nmRelation+="/";
					nmRelation+=LevelDetectTargetFlag_GetName(buf[i]);
				}
			}
		}
		if (!nmRelation.empty())
			nmRelation=std::string("[")+nmRelation+"]";
	}
	return nmRelation.c_str();
}

inline const char *LevelDetectTargetFlags_GetTypeName(LevelDetectTargetFlag flags)
{
	static std::string nmType;
	nmType="";
	LevelDetectTargetFlag flagsType=(LevelDetectTargetFlag)(flags&LevelDetectTargetFlag_Type);
	if (TRUE)
	{
		LevelDetectTargetFlag buf[]={LevelDetectTarget_Unit,LevelDetectTarget_Player,LevelDetectTarget_Agent,LevelDetectTarget_Item};
		for (int i=0;i<ARRAY_SIZE(buf);i++)
		{
			if (flagsType&buf[i])
			{
				if (!nmType.empty())
					nmType+="/";
				nmType+=LevelDetectTargetFlag_GetName(buf[i]);
			}
		}
		if (!nmType.empty())
			nmType=std::string("[")+nmType+"]";
	}
	return nmType.c_str();
}


inline const char *LevelDetectTargetFlags_GetMethodName(LevelDetectTargetFlag flags)
{
	static std::string nmMethod;
	nmMethod="";
	LevelDetectTargetFlag flagsMethod=(LevelDetectTargetFlag)(flags&LevelDetectTargetFlag_Method);
	if (TRUE)
	{
		LevelDetectTargetFlag buf[]={LevelDetectTarget_Ground,LevelDetectTarget_Resided,LevelDetectTarget_Flying};
		BOOL bAll=TRUE;
		for (int i=0;i<ARRAY_SIZE(buf);i++)
		{
			if (flagsMethod&buf[i])
			{
				if (!nmMethod.empty())
					nmMethod+="/";
				nmMethod+=LevelDetectTargetFlag_GetName(buf[i]);
			}
			else
				bAll=FALSE;
		}
		if (!nmMethod.empty())
			nmMethod=std::string("[")+nmMethod+"]";
		if (bAll)
			nmMethod="";
	}
	return nmMethod.c_str();
}




inline const char *LevelDetectTargetFlags_GetName(LevelDetectTargetFlag flags)
{
	static std::string nm;
	nm="n/a";
	LevelDetectTargetFlag flagsRelation=(LevelDetectTargetFlag)(flags&LevelDetectTargetFlag_Relation);
	LevelDetectTargetFlag flagsType=(LevelDetectTargetFlag)(flags&LevelDetectTargetFlag_Type);
	LevelDetectTargetFlag flagsMethod=(LevelDetectTargetFlag)(flags&LevelDetectTargetFlag_Method);
	if (flagsRelation==0)
	{
		if ((flags&(LevelDetectTarget_Unit|LevelDetectTarget_Player|LevelDetectTarget_Agent))!=0)
			return nm.c_str();
	}
	if (flagsMethod==0)
	{
		if ((flags&(LevelDetectTarget_Unit|LevelDetectTarget_Player))!=0)
			return nm.c_str();
	}
	if (flagsType==0)
		return nm.c_str();

	std::string nmRelation=LevelDetectTargetFlags_GetRelationName(flags);
	std::string nmType=LevelDetectTargetFlags_GetTypeName(flags);
	std::string nmMethod=LevelDetectTargetFlags_GetMethodName(flags);

	if (!nmRelation.empty())
		nm=nmRelation+"的"+nmMethod+nmType;
	else
		nm=nmMethod+nmType;
	return nm.c_str();
}

inline const char *LevelDetectTargetFlags_GetName(std::vector<LevelDetectTargetFlag> flags,StringID nmRef)
{
	static std::string nm;
	nm="";
	if (nmRef==0xffffffff)//StringID_BhvValInvalidRef
	{
		if (flags.size()>0)
		{
			for (int i=0;i<flags.size();i++)
			{
				if (!nm.empty())
					nm=nm+"\n";
				nm=nm+LevelDetectTargetFlags_GetName(flags[i]);
			}
		}
	}
	else
	{
		extern const char *StrLib_GetStr(StringID id);
		nm=std::string("[")+StrLib_GetStr(nmRef)+"]";
	}

	return nm.c_str();
}

