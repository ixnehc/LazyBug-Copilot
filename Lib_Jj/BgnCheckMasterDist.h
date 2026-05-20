#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_CheckMasterDist:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckMasterDist);

	virtual const char *GetTypeName()	{		return "检测与主人距离";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"范围内");
			STUB_OUT(2,"范围外");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"判断与主人是否在%.2f米范围内",radius);
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckMasterDist,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(float,radius,5.0f);
			GELEM_EDITVAR("范围半径",GVT_F,GSem(GSem_Float,"0,100,0.1"),"检测范围");
	END_GOBJ();    

public: //当作protected

	float radius;

};

class CBgn_CheckMasterDist:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckMasterDist);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
