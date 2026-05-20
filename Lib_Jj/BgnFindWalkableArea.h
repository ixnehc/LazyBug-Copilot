#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_FindWalkableArea:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_FindWalkableArea);

	virtual const char *GetTypeName()	{		return "寻找可走区域";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"找到");
			STUB_OUT(2,"未找到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (nmOutput==StringID_Invalid)
			s="n/a";
		else
		{
			std::string s1=GetBVRDesc_Float(BVR_ARG(distFwd),assist);
			std::string s2=GetBVRDesc_Float(BVR_ARG(radius),assist);
			FormatString(s,"在前方%s米处,找一块半径为%s的可走区域,中心点保存在变量%s中",
				s1.c_str(),s2.c_str(),
				assist->GetStr(nmOutput));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_FindWalkableArea,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(float,radius,4.0f);
			GELEM_EDITVAR("区域半径",GVT_F,GSem(GSem_Float,"0,20,0.01"),"区域半径");
			GELEM_BVR();
		GELEM_VAR_INIT(float,distFwd,3.0f);
			GELEM_EDITVAR("前方距离",GVT_F,GSem(GSem_Float,"0,20,0.01"),"前方距离");
			GELEM_BVR();
		GELEM_BEHAVIORMEM_POS(nmOutput,"保存变量","结果保存到哪个变量中")
	END_GOBJ();    

public: //当作protected
	DEFINE_BVR(float,radius);
	DEFINE_BVR(float,distFwd);

	StringID nmOutput;
};


class CBgn_FindWalkableArea:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_FindWalkableArea);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


