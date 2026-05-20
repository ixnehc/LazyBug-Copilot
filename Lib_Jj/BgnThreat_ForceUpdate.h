#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpThreat_ForceUpdate:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_ForceUpdate);

	virtual const char *GetTypeName()	{		return "强制更新Threat";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Threat;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="强制更新Threat";
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_ForceUpdate,1);
		GELEM_BGP_BASE();
    END_GOBJ();    

public: //当作protected

};


struct LevelRecordSkill;
class CBgnThreat_ForceUpdate:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_ForceUpdate);

	CBgnThreat_ForceUpdate()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:


};

