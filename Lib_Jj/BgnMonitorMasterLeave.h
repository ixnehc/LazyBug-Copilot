#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_MonitorMasterLeave:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_MonitorMasterLeave);

	virtual const char *GetTypeName()	{		return "监控主人是否远离";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"远离");
			STUB_OUT(2,"中止");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"判断主人是否在远离自己");
	}

    BEGIN_GOBJ_PURE_UID(CBgp_MonitorMasterLeave,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(float,radius,4.0f);
			GELEM_EDITVAR("范围半径",GVT_F,GSem(GSem_Float,"0,100,0.1"),"多大范围外的远离才算远离");
	END_GOBJ();    

public: //当作protected

	float radius;

};

class CBgn_MonitorMasterLeave:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_MonitorMasterLeave);
	CBgn_MonitorMasterLeave()
	{
		_dist=0.0f;
		_distLeave=0.0f;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:
	LevelPos _posMe;
	LevelPos _posMaster;
	float _dist;

	float _distLeave;//累积的离开距离

};
