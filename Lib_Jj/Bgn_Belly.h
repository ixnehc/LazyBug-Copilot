#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgp_BellyMinion_RequestAction:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_BellyMinion_RequestAction);

	virtual const char *GetTypeName()	{		return "BellyMinion_RequestAction";	}
	virtual DWORD GetStubCount()
	{
		return 4;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"NoAction");
			STUB_OUT(2,"Hop");
			STUB_OUT(3,"StompEgg");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="请求Action";
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_BellyMinion_RequestAction,490,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_POS(nmVarPos,"位置保存变量","结果保存到哪个变量中")
		GELEM_BEHAVIORMEM_OBJID(nmVarObjID,"对象ID保存变量","结果保存到哪个变量中")
    END_GOBJ();    

public: //当作protected
	StringID nmVarPos;
	StringID nmVarObjID;


};


class CBgn_BellyMinion_RequestAction:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_BellyMinion_RequestAction);

	CBgn_BellyMinion_RequestAction()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:
};



class CBgp_BellyKing_RequestPos:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_BellyKing_RequestPos);

	virtual const char *GetTypeName()	{		return "BellyKing_RequestPos";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (tp==0)
			s="请求SpawnEgg的位置";
		if (tp==1)
			s="请求EvadeJump的位置";
		if (tp==2)
			s="请求Approach的位置";
	}

	BEGIN_GOBJ_PURE_UID2(CBgp_BellyKing_RequestPos,491,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(DWORD,tp,0);
			GELEM_EDITVAR("位置类型",GVT_U,GSem(GSem_Interger,"SpawnEgg,EvadeJump,Approach"),"位置类型");
		GELEM_VAR_INIT(float,rangeMin,5.0f);
			GELEM_EDITVAR("最小距离",GVT_F,GSem(GSem_Float,"0.1,20.0,0.05"),"最小距离");
		GELEM_VAR_INIT(float,rangeMax,10.0f);
			GELEM_EDITVAR("最大距离",GVT_F,GSem(GSem_Float,"0.1,20.0,0.05"),"最大距离");
		GELEM_BEHAVIORMEM_POS(nmVarPos,"位置保存变量","结果保存到哪个变量中")
	END_GOBJ();    

public: //当作protected
	int tp;
	StringID nmVarPos;
	float rangeMin;
	float rangeMax;

};


class CBgn_BellyKing_RequestPos:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_BellyKing_RequestPos);

	CBgn_BellyKing_RequestPos()
	{
	}

	void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:
};


class CBgp_BellyEelOp:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_BellyEelOp);

	virtual const char *GetTypeName()	{		return "BellyEelOp";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (tp == 0)
			s = "设置SkillTarget(攻击模式)";
		if (tp == 1)
			s = "设置SkillTarget(Pacing模式)";
		if (tp == 2)
			s = "设置SkillTarget(驻留模式)";
		if (tp == 3)
			s = "设置SkillTarget(Repair模式)";
		if (tp == 4)
			s = "检测能否Repair到SkillTarget的连接";
	}

	BEGIN_GOBJ_PURE_UID2(CBgp_BellyEelOp,496,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(DWORD,tp,0);
			GELEM_EDITVAR("类型",GVT_U,GSem(GSem_Interger,"设置SkillTarget(攻击模式),设置SkillTarget(Pacing模式),设置SkillTarget(驻留模式),设置SkillTarget(Repair模式),Repair检测"),"更新类型");
	END_GOBJ();    

public: //当作protected
	int tp;

};


class CBgn_BellyEelOp:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_BellyEelOp);

	CBgn_BellyEelOp()
	{
	}

	void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:
};