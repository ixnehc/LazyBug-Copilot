#pragma once

#include "BehaviorGraphPads.h"
#include "Behavior.h"

#include "bitset/bitset.h"



class CBgp_Relay:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Relay);

	virtual const char *GetTypeName()	{		return "中继";	};
	virtual DWORD GetStubCount()	{		return 1;	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_OUT(0,"触发");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_nm==StringID_Invalid)
			s="n/a";
		else
			s=StrLib_GetStr(_nm);
	}


    BEGIN_GOBJ_PURE_UID(CBgp_Relay,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "中继名称", GVT_U, GSem(GSem_StringID,"行为图中继名称"), "中继的名称" );

    END_GOBJ();    

public: //当作protected
	StringID _nm;
};


class CBgn_Relay:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Relay);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
};



class CBgp_StartRelay:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_StartRelay);

	virtual const char *GetTypeName()	{		return "启动中继";	};
	virtual DWORD GetStubCount()
	{
		return 1;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_nm!=StringID_Invalid)
			s=s+StrLib_GetStr(_nm);
		else
			s="n/a";
	}


    BEGIN_GOBJ_PURE_UID(CBgp_StartRelay,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "中继名称", GVT_U, GSem(GSem_StringID,"行为图中继名称"), "中继的名称" );

    END_GOBJ();    

public: //当作protected
	StringID _nm;
};


class CBgn_StartRelay:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_StartRelay);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

};

