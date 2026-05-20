#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_地狱触手_侦测干扰:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_地狱触手_侦测干扰);

	virtual const char *GetTypeName()	{		return "地狱触手_侦测干扰";	}
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

    BEGIN_GOBJ_PURE_UID2(CBgp_地狱触手_侦测干扰,412,1);
		GELEM_BGP_BASE();
			GELEM_VAR_INIT(float,_radius,2.0f);
				GELEM_EDITVAR("侦测半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"侦测半径");

    END_GOBJ();    

public: //当作protected
	float _radius;

};


class CBgn_地狱触手_侦测干扰:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_地狱触手_侦测干扰);

	CBgn_地狱触手_侦测干扰()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

	BOOL _Check(CLevelObj *lo,float dist2);

};

