#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckStunInfo:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckStunInfo);

	enum CheckType
	{
		Check_None,
		Check_BrokenWeaks,
		Check_StunSrcDist,
		Check_StrikeFromLeft,

		Check_ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "检测Stun信息";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Buff;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist);

    BEGIN_GOBJ_PURE_UID(CBgp_CheckStunInfo,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(CheckType,_tpCheck,Check_BrokenWeaks);
			GELEM_EDITVAR("检测类型",GVT_S,GSem(GSem_Interger,
				"被击破弱点:1"		"|最小距离&最大距离,"
				"硬直Src距离:2"	"|击破弱点,"
				"Strike来自左侧:3"	"|击破弱点&最小距离&最大距离"
				),"奖励类型");
		GELEM_OBJ(WeaksEx,_weaksBroken);
			GELEM_EDITOBJ("击破弱点","击破弱点");
		GELEM_VAR_INIT(float,_distMin,0.0f);
			GELEM_EDITVAR("最小距离",GVT_F,GSem(GSem_Float,"0.0,10000.0,0.05"),"最小距离");
		GELEM_VAR_INIT(float,_distMax,10.0f);
			GELEM_EDITVAR("最大距离",GVT_F,GSem(GSem_Float,"0.0,10000.0,0.05"),"最大距离");
	END_GOBJ();    

public: //当作protected
	CheckType _tpCheck;
	WeaksEx _weaksBroken;
	float _distMin;
	float _distMax;
};


class CBgn_CheckStunInfo:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckStunInfo);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
