/********************************************************************
	created:	2014/10/05 
	author:		cxi
	
	purpose:	BehaviorParam
*********************************************************************/
#include "stdh.h"
#include "anim/animdefines.h"
#include "BehaviorParam.h"
#include "FillDescAssist.h"
#include "../stringparser/stringparser.h"
#include "BehaviorCustomConst.h"
#include "BehaviorValue.h"

static const char *GetVarName(StringID id,FillDescAssist *assist)
{
	static std::string s;
	if (id==StringID_Invalid)
		return "null";
	FormatString(s," %s ",assist->GetStr(id));
	return s.c_str();
}

const char *GetBPRDesc(BPR_Bool &bpr,FillDescAssist *assist)
{
	if (!bpr.bRef)
		return bpr.v?"TRUE":"FALSE";

	return GetVarName(bpr.nmRef,assist);
}


const char *GetBPRDesc(BPR_Int &bpr,FillDescAssist *assist)
{
	static std::string s;
	if (!bpr.bRef)
	{
		FormatString(s,"%d",bpr.v);
		return s.c_str();
	}
	return GetVarName(bpr.nmRef,assist);
}

const char *GetBPRDesc(BPR_Float &bpr,FillDescAssist *assist)
{
	static std::string s;
	if (!bpr.bRef)
	{
		FormatString(s,"%.3f",bpr.v);
		return s.c_str();
	}

	return GetVarName(bpr.nmRef,assist);
}

const char *GetBPRDesc(BPR_SkillID &bpr,FillDescAssist *assist)
{
	if (!bpr.bRef)
		return assist->GetSkillName(bpr.v);

	return GetVarName(bpr.nmRef,assist);
}

const char *GetBPRDesc(BPR_BuffID &bpr,FillDescAssist *assist)
{
	if (!bpr.bRef)
		return assist->GetBuffName(bpr.v);

	return GetVarName(bpr.nmRef,assist);
}

const char *GetBPRDesc(BPR_ItemID &bpr,FillDescAssist *assist)
{
	if (!bpr.bRef)
		return assist->GetItemName(bpr.v);

	return GetVarName(bpr.nmRef,assist);
}

const char *GetBPRDesc(BPR_UnitID &bpr,FillDescAssist *assist)
{
	if (!bpr.bRef)
		return assist->GetUnitName(bpr.v);

	return GetVarName(bpr.nmRef,assist);
}

const char *GetBPRDesc(BPR_ResourceID &bpr,FillDescAssist *assist)
{
	if (!bpr.bRef)
		return assist->GetResName(bpr.v);

	return GetVarName(bpr.nmRef,assist);
}

const char *GetBPRDesc(BPR_EoID &bpr,FillDescAssist *assist)
{
	if (!bpr.bRef)
		return assist->GetEoName(bpr.v);

	return GetVarName(bpr.nmRef,assist);
}


const char *GetBPRDesc(BPR_StringID &bpr,FillDescAssist *assist)
{
	if (!bpr.bRef)
		return assist->GetStr(bpr.v);

	return GetVarName(bpr.nmRef,assist);
}

const char *GetBPRDesc(BPR_Custom &bpr,FillDescAssist *assist)
{
	return GetVarName(bpr.nmRef,assist);
}

//XXXXX:more BehaviorMemType

//XXXXX:more BPR

const char *GetBVRDesc_Bool(BOOL v,StringID nmRef,FillDescAssist *assist)
{
	static std::string s;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		s=v?"True":"False";
		return s.c_str();
	}
	return GetVarName(nmRef,assist);
}

const char *GetBVRDesc_Int(int v,StringID nmRef,FillDescAssist *assist)
{
	static std::string s;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		FormatString(s,"%d",v);
		return s.c_str();
	}
	return GetVarName(nmRef,assist);
}


const char *GetBVRDesc_Float(float v,StringID nmRef,FillDescAssist *assist)
{
	static std::string s;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		FormatString(s,"%.3f",v);
		return s.c_str();
	}
	return GetVarName(nmRef,assist);
}

const char *GetBVRDesc_String(std::string &ss,StringID nmRef,FillDescAssist *assist)
{
	static std::string s;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		FormatString(s,"\"%s\"",ss.c_str());
		return s.c_str();
	}
	return GetVarName(nmRef,assist);
}



const char *GetBVRDesc_StringID(StringID v,StringID nmRef,FillDescAssist *assist)
{
	static std::string s;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		if (v==StringID_Invalid)
			s="null";
		else
			FormatString(s,"%s",assist->GetStr(v));
		return s.c_str();
	}
	return GetVarName(nmRef,assist);
}



const char *GetBVRDesc_SkillID(RecordID v,StringID nmRef,FillDescAssist *assist)
{
	static std::string s;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		FormatString(s,"%s",assist->GetSkillName(v));
		return s.c_str();
	}
	return GetVarName(nmRef,assist);
}

const char *GetBVRDesc_ItemID(RecordID v,StringID nmRef,FillDescAssist *assist)
{
	static std::string s;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		FormatString(s,"%s",assist->GetItemName(v));
		return s.c_str();
	}
	return GetVarName(nmRef,assist);
}

const char *GetBVRDesc_BuffID(RecordID v,StringID nmRef,FillDescAssist *assist)
{
	static std::string s;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		FormatString(s,"%s",assist->GetBuffName(v));
		return s.c_str();
	}
	return GetVarName(nmRef,assist);
}

const char *GetBVRDesc_UnitID(RecordID v,StringID nmRef,FillDescAssist *assist)
{
	static std::string s;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		FormatString(s,"%s",assist->GetUnitName(v));
		return s.c_str();
	}
	return GetVarName(nmRef,assist);
}

const char *GetBVRDesc_EoID(RecordID v,StringID nmRef,FillDescAssist *assist)
{
	static std::string s;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		FormatString(s,"%s",assist->GetEoName(v));
		return s.c_str();
	}
	return GetVarName(nmRef,assist);
}

const char *GetBVRDesc_ResourceID(RecordID v,StringID nmRef,FillDescAssist *assist)
{
	static std::string s;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		FormatString(s,"%s",assist->GetResName(v));
		return s.c_str();
	}
	return GetVarName(nmRef,assist);
}

const char *GetBVRDesc_AnimTick(AnimTick v,StringID nmRef,FillDescAssist *assist)
{
	static std::string s;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		FormatString(s,"%.3f",ANIMTICK_TO_SECOND(v));
		return s.c_str();
	}
	return GetVarName(nmRef,assist);
}
//XXXXX:more BehaviorMemType
