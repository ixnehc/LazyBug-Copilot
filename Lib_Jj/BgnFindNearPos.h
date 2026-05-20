#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_FindNearPos:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_FindNearPos);

	virtual const char *GetTypeName()	{		return "寻找临近位点";	}
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
			if (nmCenter!=StringID_Invalid)
			{
				FormatString(s,"以%s为中心,%s为半径(+/-%.2f),寻找位点,结果保存在变量%s中",
					assist->GetStr(nmCenter),
					GetBVRDesc_Float(BVR_ARG(radius),assist),radiusVary,
					assist->GetStr(nmOutput));
			}
			else
			{
				FormatString(s,"以自己为中心,%s为半径(+/-%.2f),寻找位点,结果保存在变量%s中",
					GetBVRDesc_Float(BVR_ARG(radius),assist),radiusVary,
					assist->GetStr(nmOutput));
			}
			AppendFmtString(s,"\n尝试%s次",GetBVRDesc_Int(BVR_ARG(nTry),assist));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_FindNearPos,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(float,radius,1.0f);
			GELEM_EDITVAR("寻找半径",GVT_F,GSem(GSem_Float,"0,20,0.01"),"寻找半径");
			GELEM_BVR();
		GELEM_VAR_INIT(float,radiusVary,0.0f);
			GELEM_EDITVAR("寻找半径浮动值",GVT_F,GSem(GSem_Float,"0,20,0.01"),"寻找半径浮动值");
		GELEM_VAR_INIT(BOOL,bWalkable,TRUE);
			GELEM_EDITVAR("是否要求可走",GVT_S,GSem_Boolean,"是否要求可走");
			GELEM_BVR();
		GELEM_VAR_INIT(int,nTry,5);
			GELEM_EDITVAR("尝试次数",GVT_S,GSem_Interger,"尝试次数");
			GELEM_BVR();
		GELEM_BEHAVIORMEM_POS(nmCenter,"中心点","从哪个变量中取得中心点位置")
		GELEM_BEHAVIORMEM_POS(nmOutput,"保存变量","结果保存到哪个变量中")
	END_GOBJ();    

public: //当作protected
	DEFINE_BVR(float,radius);
	float radiusVary;
	DEFINE_BVR(int,nTry);
	DEFINE_BVR(BOOL,bWalkable);
	StringID nmCenter;
	StringID nmOutput;
};


class CBgn_FindNearPos:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_FindNearPos);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


