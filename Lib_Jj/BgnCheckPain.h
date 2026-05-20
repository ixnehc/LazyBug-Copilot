#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckPain:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckPain);

	virtual const char *GetTypeName()	{		return "检测Pain";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"是");
			STUB_OUT(2,"否");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if ((rateMin>=1.0f)&&(rateMax>=1.0f))
			FormatString(s,"检测当前Pain是否已满");
		else
			FormatString(s,"检测当前Pain的百分比是否介于%.1f~%.1f之间",rateMin*100.0f,rateMax*100.0f);
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_CheckPain,484,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(float,rateMin,0.0f);
			GELEM_EDITVAR("百分比(最小)",GVT_F,GSem(GSem_Float,"0,1,0.01"),"百分比(最小)");
		GELEM_VAR_INIT(float,rateMax,0.5f);
			GELEM_EDITVAR("百分比(最大)",GVT_F,GSem(GSem_Float,"0,1,0.01"),"百分比(最大)");
	END_GOBJ();    

public: //当作protected
	float rateMin;
	float rateMax;

};


class CBgn_CheckPain:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckPain);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
