#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelEvents.h"




class CBgp_CheckEventSrc:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckEventSrc);

	virtual const char *GetTypeName()	{		return "检测事件";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"检测到");
			STUB_OUT(2,"未检测到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		extern const char *GetBVRDesc_AnimTick(AnimTick v,StringID nmRef,FillDescAssist *assist);

		if (mode==CheckMode_MeFromAnyone)
		{
			if (!bWait)
				FormatString(s,"检测%s秒内的事件",GetBVRDesc_AnimTick(BVR_ARG(durGap),assist));
			else
				FormatString(s,"持续检测%s秒内的事件",GetBVRDesc_AnimTick(BVR_ARG(durGap),assist));
			if (nmFromResult!=StringID_Invalid)
				AppendFmtString(s,"\n发送者保存在[%s]中",StrLib_GetStr(nmFromResult));
		}
		if (mode==CheckMode_MeFromThreat)
		{
			if (!bWait)
				FormatString(s,"检测%s秒内收到来自Threat的事件",GetBVRDesc_AnimTick(BVR_ARG(durGap),assist));
			else
				FormatString(s,"持续检测%s秒内来自Threat的事件",GetBVRDesc_AnimTick(BVR_ARG(durGap),assist));
		}
		if (mode==CheckMode_ThreatFromMe)
		{
			if (!bWait)
				FormatString(s,"检测BestThreat%s秒内收到的来自自己的事件",GetBVRDesc_AnimTick(BVR_ARG(durGap),assist));
			else
				FormatString(s,"持续检测BestThreat%s秒内收到的来自自己的事件",GetBVRDesc_AnimTick(BVR_ARG(durGap),assist));
		}
		if (mode==CheckMode_CustomFromAnyone)
		{
			if (nmTo==StringID_Invalid)
				s="n/a";
			else
			{
				if (!bWait)
					FormatString(s,"检测[%s]在%s秒内的事件",StrLib_GetStr(nmTo),GetBVRDesc_AnimTick(BVR_ARG(durGap),assist));
				else
					FormatString(s,"持续检测[%s]在%s秒内的事件",StrLib_GetStr(nmTo),GetBVRDesc_AnimTick(BVR_ARG(durGap),assist));
				if (nmFromResult!=StringID_Invalid)
					AppendFmtString(s,"\n发送者保存在[%s]中",StrLib_GetStr(nmFromResult));
			}
		}
		if (mode==CheckMode_CustomFromMe)
		{
			if (nmTo==StringID_Invalid)
				s="n/a";
			else
			{
				if (!bWait)
					FormatString(s,"检测[%s]在%s秒内收到的来自自己的事件",StrLib_GetStr(nmTo),GetBVRDesc_AnimTick(BVR_ARG(durGap),assist));
				else
					FormatString(s,"持续检测[%s]在%s秒内收到的来自自己的事件",StrLib_GetStr(nmTo),GetBVRDesc_AnimTick(BVR_ARG(durGap),assist));
			}
		}
		if (mode==CheckMode_CustomFromCustom)
		{
			if ((nmFrom==StringID_Invalid)||(nmTo==StringID_Invalid))
				s="n/a";
			else
			{
				if (!bWait)
					FormatString(s,"检测[%s]在%s秒内收到来自[%s]的事件",StrLib_GetStr(nmTo),GetBVRDesc_AnimTick(BVR_ARG(durGap),assist),StrLib_GetStr(nmFrom));
				else
					FormatString(s,"持续检测[%s]在%s秒内收到来自[%s]的事件",StrLib_GetStr(nmTo),GetBVRDesc_AnimTick(BVR_ARG(durGap),assist),StrLib_GetStr(nmFrom));
			}
		}

	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckEventSrc,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(DWORD,mode,CheckMode_MeFromAnyone);

			GELEM_EDITVAR("工作模式",GVT_U,GSem(GSem_Interger,
				"检测自己收到的事件"			"|收到事件者&发送事件者,"
				"检测Threat收到来自自己的事件"			"|收到事件者&发送事件者&发送事件者保存变量,"
				"检测自己收到的来自Threat的事件"			"|收到事件者&发送事件者&发送事件者保存变量,"
				"检测指定单位收到的来自指定单位的事件"			"|发送事件者保存变量,"
				"检测指定单位收到的事件"			"|发送事件者,"
				"检测指定单位收到的来自自己的事件"			"|发送事件者&发送事件者保存变量"
				),"工作模式");
		GELEM_BEHAVIORMEM_OBJID(nmTo,"收到事件者","收到事件者");
		GELEM_BEHAVIORMEM_OBJID(nmFrom,"发送事件者","发送事件者");
		GELEM_BEHAVIORMEM_OBJID(nmFromResult,"发送事件者保存变量","发送事件者保存变量");
		GELEM_VAR_INIT(LevelEventTypeMask,maskType,12);
			GELEM_EDITVAR("事件类型",GVT_U,GSem(GSem_Flags,"Damage:4,Hit:8"),"检测什么类型的事件");
		GELEM_VAR_INIT(AnimTick,durGap,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("多长时间之前",GVT_U,GSem(GSem_AnimTick,"0,20,0.1"),"多长时间之前");
			GELEM_BVR();
		GELEM_VAR_INIT(BOOL,bWait,TRUE);
			GELEM_EDITVAR("持续检测",GVT_S,GSem_Boolean,"持续检测直至检测到");
	END_GOBJ();    

public: //当作protected

	enum CheckMode
	{
		CheckMode_MeFromAnyone,
		CheckMode_ThreatFromMe,
		CheckMode_MeFromThreat,
		CheckMode_CustomFromCustom,
		CheckMode_CustomFromAnyone,
		CheckMode_CustomFromMe,

		ForceDword=0xffffffff,
	};

	CheckMode mode;

	LevelEventTypeMask maskType;
	DEFINE_BVR(AnimTick,durGap);
	BOOL bWait;
	StringID nmFrom;
	StringID nmTo;
	StringID nmFromResult;

};


class CBgn_CheckEventSrc:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckEventSrc);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:
	BOOL _Update(BGNOutputs &outputs);
};
