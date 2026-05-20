#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgp_StopMove:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_StopMove);

	virtual const char *GetTypeName()	{		return "停止移动";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="停止移动\n如果当前有技能在施放,则等待它完成";
	}

    BEGIN_GOBJ_PURE_UID(CBgp_StopMove,1);
		GELEM_BGP_BASE();
    END_GOBJ();    

public: //当作protected


};


class CBgn_StopMove:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_StopMove);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:


};
