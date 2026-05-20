#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpThreat_Combating:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_Combating);

	virtual const char *GetTypeName()	{		return "对Threat进行Combat过程";	}
	virtual DWORD GetStubCount()
	{
		return 1;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Threat;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="对Threat进行Combat过程";
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_Combating,1);
		GELEM_BGP_BASE();
    END_GOBJ();    

public: //当作protected

};


struct LevelRecordSkill;
class CBgnThreat_Combating:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_Combating);

	CBgnThreat_Combating()
	{
		_threat=NULL;
	}



	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:

	CLevelObj *_threat;
	LevelObjID _idThreat;

};

