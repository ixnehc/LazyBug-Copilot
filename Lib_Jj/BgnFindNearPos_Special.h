#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_FindNearPos_Special:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_FindNearPos_Special);

	enum Mode
	{
		DodgeAttackThreat,
		ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "寻找临近位点(特需)";	}
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
			FormatString(s,"以%s为半径,寻找位点,结果保存在变量%s中",
				GetBVRDesc_Float(BVR_ARG(radius),assist),
				assist->GetStr(nmOutput));
			AppendFmtString(s,"\n尝试%s次",GetBVRDesc_Int(BVR_ARG(nTry),assist));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_FindNearPos_Special,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(DWORD,mode,DodgeAttackThreat);
			GELEM_EDITVAR("工作模式",GVT_U,GSem(GSem_Interger,"寻找闪躲攻击的位置"),"工作模式");
		GELEM_VAR_INIT(float,radius,4.0f);
			GELEM_EDITVAR("寻找半径",GVT_F,GSem(GSem_Float,"0,20,0.01"),"寻找半径");
			GELEM_BVR();
		GELEM_VAR_INIT(float,radiusVary,1.0f);
			GELEM_EDITVAR("寻找半径浮动值",GVT_F,GSem(GSem_Float,"0,20,0.01"),"寻找半径浮动值");
			GELEM_BVR();
		GELEM_VAR_INIT(float,radiusToThreat,6.0f);
			GELEM_EDITVAR("到Threat的距离限制",GVT_F,GSem(GSem_Float,"0,20,0.01"),"到Threat的距离限制");
			GELEM_BVR();
		GELEM_VAR_INIT(float,radiusToThreatVary,3.0f);
			GELEM_EDITVAR("到Threat的距离限制浮动值",GVT_F,GSem(GSem_Float,"0,20,0.01"),"到Threat的距离限制浮动值");
			GELEM_BVR();
		GELEM_VAR_INIT(int,nTry,5);
			GELEM_EDITVAR("尝试次数",GVT_S,GSem_Interger,"尝试次数");
			GELEM_BVR();
		GELEM_BEHAVIORMEM_POS(nmOutput,"保存变量","结果保存到哪个变量中")
	END_GOBJ();    

public: //当作protected
	Mode mode;
	DEFINE_BVR(float,radius);
	DEFINE_BVR(float,radiusVary);
	DEFINE_BVR(float,radiusToThreat);
	DEFINE_BVR(float,radiusToThreatVary);
	DEFINE_BVR(int,nTry);

	StringID nmOutput;
};


class CBgn_FindNearPos_Special:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_FindNearPos_Special);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


