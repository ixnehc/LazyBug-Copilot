#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckJumpAttack:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckJumpAttack);

	virtual const char *GetTypeName()	{		return "跳跃攻击检测";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"检测是否可以跳跃攻击:\n距离介于%.2f~%.2f之间",rangeMin,rangeMax);
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckJumpAttack,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_OBJID(nmVar,"检测对象变量","检查哪个对象")
		GELEM_VAR_INIT(float,rangeMin,3.0f);
			GELEM_EDITVAR("最小距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"多远距离以外的单位");
		GELEM_VAR_INIT(float,rangeMax,5.0f);
			GELEM_EDITVAR("最大距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"多远距离以内的单位");
	END_GOBJ();    

public: //当作protected
	StringID nmVar;
	float rangeMin;
	float rangeMax;

};


class CBgn_CheckJumpAttack:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckJumpAttack);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
