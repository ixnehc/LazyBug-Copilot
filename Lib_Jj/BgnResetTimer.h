#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgp_ResetTimer_Obsolete:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_ResetTimer_Obsolete);

	virtual const char *GetTypeName()	{		return "重置计时器(Obsolete)";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (nm!=StringID_Invalid)
		{
			FormatString(s,"重置计时器(%s)",StrLib_GetStr(nm));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_ResetTimer_Obsolete,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"行为图计时器名称"), "行为图计时器名称" );
    END_GOBJ();    

public: //当作protected

	StringID nm;
};


class CBgn_ResetTimer_Obsolete:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_ResetTimer_Obsolete);

	CBgn_ResetTimer_Obsolete()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



struct BMO_Timer:public CBehaviorMemObj
{
	DECLARE_CLASS(BMO_Timer);
	BEGIN_GOBJ_PURE(BMO_Timer,1);

		GELEM_VAR_INIT(AnimTick,tTimeUp,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_UID(1);
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_UID(2);

	END_GOBJ();

	AnimTick tTimeUp;
	AnimTick dur;
};


class CBgp_ResetTimer:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_ResetTimer);

	virtual const char *GetTypeName()	{		return "重置计时器";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_timer!=StringID_Invalid)
		{
			FormatString(s, "重置计时器(%s)", StrLib_GetStr(_timer));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_ResetTimer,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,_timer,StringID_Invalid);
			GELEM_EDITVAR( "Timer名称", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "Timer名称");
		GELEM_VAR_INIT(float, _fDur, 10.0f); GELEM_UID(1);
			GELEM_EDITVAR("Timer时间",GVT_F,GSem(GSem_Float,"0,100,0.1"),"Timer时间");
			GELEM_BVR();
		GELEM_VAR_INIT(float, _fVariance,0.0f); GELEM_UID(2);
			GELEM_EDITVAR("Timer浮动时间",GVT_F,GSem(GSem_Float,"0,100,0.1"),"Timer浮动时间");
			GELEM_BVR();
    END_GOBJ();    

public: //当作protected
	StringID _timer;
	DEFINE_BVR(float,_fDur);
	DEFINE_BVR(float, _fVariance);

};


class CBgn_ResetTimer:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_ResetTimer);

	CBgn_ResetTimer()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



