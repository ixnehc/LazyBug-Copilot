#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_地狱触手_FindAttackTarget:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_地狱触手_FindAttackTarget);

	virtual const char *GetTypeName()	{		return "地狱触手_FindAttackTarget";	}
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
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_地狱触手_FindAttackTarget,411,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_POS(varPos,"[out]位置变量","找到的位置存放在哪里")
		GELEM_VAR_INIT(BOOL,bFindClosest,FALSE);
			GELEM_EDITVAR("寻找最近的攻击点",GVT_S,GSem_Boolean,"寻找最近的攻击点");
		GELEM_VAR_INIT(float,_radius侦测干扰,2.0f);
			GELEM_EDITVAR("侦测干扰半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"侦测干扰半径");
    END_GOBJ();    

public: //当作protected
	StringID varPos;
	BOOL bFindClosest;
	float _radius侦测干扰;

};


class CBgn_地狱触手_FindAttackTarget:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_地狱触手_FindAttackTarget);

	CBgn_地狱触手_FindAttackTarget()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

