#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckTimer_Obsolete:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckTimer_Obsolete);

	virtual const char *GetTypeName()	{		return "检测计时器(obsolete)";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"时间到");
			STUB_OUT(2,"时间没到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (nm==StringID_Invalid)
			s="n/a";
		else
		{
			FormatString(s,"计时器(%s)时间有没有到?",StrLib_GetStr(nm));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckTimer_Obsolete,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"行为图计时器名称"), "行为图计时器名称" );
	END_GOBJ();    

public: //当作protected
	StringID nm;
};


class CBgn_CheckTimer_Obsolete:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_CheckTimer_Obsolete);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



class CBgp_CheckTimer:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckTimer);


	virtual const char *GetTypeName()	{		return "检测计时器";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"时间到");
			STUB_OUT(2,"时间没到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_timer!=StringID_Invalid)
		{
			FormatString(s,"检测计时器(%s)时间有没有到",StrLib_GetStr(_timer));
			if (_durAdd>0.0f)
				AppendFmtString(s,"(附加%.2f秒)",_durAdd);
		}
	}

	BEGIN_GOBJ_PURE_UID(CBgp_CheckTimer,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,_timer,StringID_Invalid);
			GELEM_EDITVAR( "Timer名称", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "Timer名称");
		GELEM_VAR_INIT(float,_durAdd,0.0f);
			GELEM_EDITVAR("附加时间",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.05"),"附加时间");
	END_GOBJ();    

public: //当作protected
	StringID _timer;
	float _durAdd;
};


class CBgn_CheckTimer:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckTimer);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

