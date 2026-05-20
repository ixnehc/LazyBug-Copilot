#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgp_SetUnitPace:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_SetUnitPace);

	virtual const char *GetTypeName()	{		return "设置Pace";	}
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
		if (bCustomPace)
			s="设置自定义Pace";
		else
			s="设置缺省Pace";
	}

    BEGIN_GOBJ_PURE_UID(CBgp_SetUnitPace,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(BOOL,bCustomPace,TRUE);
			GELEM_EDITVAR("自定义Pace",GVT_S,GSem(GSem_Boolean,"均数,标准差"),"自定义Pace");
		GELEM_OBJ(LevelUnitPace,pace);
			GELEM_EDITOBJ("Pace","Pace");
    END_GOBJ();    

public: //当作protected

	BOOL bCustomPace;
	LevelUnitPace pace;

};


struct LevelRecordSkill;
class CBgn_SetUnitPace:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SetUnitPace);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:
};


