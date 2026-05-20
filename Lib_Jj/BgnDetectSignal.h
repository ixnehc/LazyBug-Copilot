#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_DetectSignal:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_DetectSignal);

	virtual const char *GetTypeName()	{		return "侦测信号";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"侦测到");
			STUB_OUT(2,"未侦测到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		extern const char *GetBgnRegName(StringID nm);

		s="n/a";
		if (nm!=StringID_Invalid)
		{
			if (!bKeepDetect)
				FormatString(s,"侦测是否收到一个信号(%s)",StrLib_GetStr(nm));
			else
				FormatString(s,"持续侦测是否收到一个信号(%s)",StrLib_GetStr(nm));
			if (varSender!=StringID_Invalid)
				AppendFmtString(s,"\n保存发送者到 %s 中",StrLib_GetStr(varSender));
		}

	}

    BEGIN_GOBJ_PURE_UID(CBgp_DetectSignal,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "信号名称", GVT_U, GSem(GSem_StringID,"信号名称"), "要侦测的信号名称" );
		GELEM_BEHAVIORMEM_OBJID(varSender,"信号发送者保存变量","将检测到的信号的发送者保存到这个变量中")
		GELEM_VAR_INIT(BOOL,bKeepDetect,FALSE);
			GELEM_EDITVAR("持续侦测",GVT_S,GSem_Boolean,"是否持续侦测");
	END_GOBJ();    

public: //当作protected
	StringID nm;

	StringID varSender;

	BOOL bKeepDetect;
};


class CBgn_DetectSignal:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_DetectSignal);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:


	BOOL _DoDetect(BGNOutputs &outputs);

};

