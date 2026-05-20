#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

class CBgp_MoveAlong:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_MoveAlong);

	virtual const char *GetTypeName()	{		return "沿路线移动";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_route==StringID_Invalid)
			s="n/a";
		else
			FormatString(s,"沿路线\"%s\"移动",StrLib_GetStr(_route));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_MoveAlong,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_route,StringID_Invalid);	
			GELEM_EDITVAR( "路线名称", GVT_U, GSem(GSem_StringID,"路线名称"), "沿哪条路线移动");

    END_GOBJ();    

public: //当作protected

	StringID _route;

};


struct LevelRoute;
class CBgn_MoveAlong:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_MoveAlong);

	CBgn_MoveAlong()
	{
		_route=NULL;
		_iCurPos=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

	void _UpdateMove(CLevelObj *lo);

	LevelRoute *_route;
	DWORD _iCurPos;
};
