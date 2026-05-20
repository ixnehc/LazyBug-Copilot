#pragma once

#include "BehaviorGraphPads.h"



class CBgp_Include:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Include);

	virtual const char *GetTypeName()	{		return "内联器";	};
	virtual DWORD GetStubCount()		{				return _stubin.size()+_stubout.size();	}
	virtual PadStub GetStub(DWORD idx)	
	{		
		if (idx<_stubin.size())
		{
			StringID nm=_stubin[idx];
			if (nm==StringID_Invalid)
				return PadStub("n/a",PadStub_In,TRUE);
			return PadStub(StrLib_GetStr(nm),PadStub_In,TRUE);
		}
		idx-=_stubin.size();
		if (idx<_stubout.size())
		{
			StringID nm=_stubout[idx];
			if (nm==StringID_Invalid)
				return PadStub("n/a",PadStub_Out,TRUE);
			return PadStub(StrLib_GetStr(nm),PadStub_Out,TRUE);
		}
		return PadStub();	
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Helper;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)	
	{
		if (_nm==StringID_Invalid)
			s="n/a";
		else
			FormatString(s,"%s",StrLib_GetStr(_nm));
	}


	BEGIN_GOBJ_PURE_UID(CBgp_Include,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "行为图名称", GVT_U, GSem(GSem_StringID,"行为图名称"), "内联的行为图的名称" );
		GELEM_VARVECTOR_INIT( StringID,_stubin,StringID_Invalid);	
//			GELEM_EDITVAR( "输入Stub", GVT_U, GSem(GSem_StringID,"行为图Stub名称"), "行为图输入的Stub" );
		GELEM_VARVECTOR_INIT( StringID,_stubout,StringID_Invalid);	
//			GELEM_EDITVAR( "输出Stub", GVT_U, GSem(GSem_StringID,"行为图Stub名称"), "行为图输出的Stub" );

	END_GOBJ();    

public: //当作protected
	StringID _nm;
	std::vector<StringID> _stubin;
	std::vector<StringID> _stubout;
};


class CBgn_Include:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Include);

protected:

};


class CBgp_StubIn:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_StubIn);

	virtual const char *GetTypeName()	{		return "输入接口";	};
	virtual DWORD GetStubCount()		{				return 1;	}
	virtual PadStub GetStub(DWORD idx)	
	{		
		BEGIN_STUB()
			STUB_OUT(0,"输出");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Helper;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)	
	{
		if (_nm==StringID_Invalid)
			s="n/a";
		else
			FormatString(s,"%s",StrLib_GetStr(_nm));
	}


	BEGIN_GOBJ_PURE_UID(CBgp_StubIn,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"行为图Stub名称"), "名称" );

	END_GOBJ();    

public: //当作protected
	StringID _nm;
};


class CBgn_StubIn:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_StubIn);

protected:

};


class CBgp_StubOut:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_StubOut);

	virtual const char *GetTypeName()	{		return "输出接口";	};
	virtual DWORD GetStubCount()		{				return 1;	}
	virtual PadStub GetStub(DWORD idx)	
	{		
		BEGIN_STUB()
			STUB_IN(0,"输入");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Helper;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)	
	{
		if (_nm==StringID_Invalid)
			s="n/a";
		else
			FormatString(s,"%s",StrLib_GetStr(_nm));
	}


	BEGIN_GOBJ_PURE_UID(CBgp_StubOut,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"行为图Stub名称"), "名称" );

	END_GOBJ();    

public: //当作protected
	StringID _nm;
};


class CBgn_StubOut:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_StubOut);

protected:

};
