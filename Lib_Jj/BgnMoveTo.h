#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

class CBgpMoveTo:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpMoveTo);

	virtual const char *GetTypeName()	{		return "移动";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_varPos!=StringID_Invalid)
		{
			FormatString(s,"移向目的地[%s]",assist->GetStr(_varPos));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpMoveTo,1);
		GELEM_BEHAVIORMEM_POS(_varPos,"位置变量","从哪个变量里取得移动目的")
		GELEM_VAR_INIT(BOOL,_bMonitorReach,TRUE);
			GELEM_EDITVAR("监控抵达",GVT_S,GSem_Boolean,"是否要监控抵达");
			GELEM_BVR();
		GELEM_VAR_INIT(float,_radiusReach,0.5f);
			GELEM_EDITVAR("抵达半径",GVT_S,GSem(GSem_Float,"0,20,0.01"),"抵达半径");
			GELEM_BVR();
    END_GOBJ();    

public: //当作protected

	StringID _varPos;
	DEFINE_BVR(BOOL,_bMonitorReach);
	DEFINE_BVR(float,_radiusReach);
};


class CBgnMoveTo:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnMoveTo);

	CBgnMoveTo()
	{
		_radiusReach=0.0f;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

	float _radiusReach;
	LevelPos _posTarget;
	DWORD _verCast;

};

