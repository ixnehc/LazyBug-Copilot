#pragma once

#include "math/range.h"

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpThreat_CheckRange:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_CheckRange);

	enum TargetType
	{
		Target_Threat,
		Target_Custom,

		Target_ForceDword=0xffffffff,
	};


	virtual const char *GetTypeName()	{		return "检测Threat范围";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Threat;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"检测%s是否\n在半径范围[ %.2f米 ~ %.2f米 ]内\n朝向范围[ %.2f度 ~ %.2f度 ]内",
			BgnLevelSkillTarget::GetDesc(BVR_ARG(_target)),
			_rngRadius.low,_rngRadius.hi,
			_rngFace.low,_rngFace.hi);
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_CheckRange,1);
		GELEM_BGP_BASE();

		GELEM_OBJ(BgnLevelSkillTarget,_target);GELEM_UID(1); 
			GELEM_EDITOBJ("目标","目标");
			GELEM_BVR();

		GELEM_VAR_INIT(BOOL,_bConsiderUnitRadius,FALSE);
			GELEM_EDITVAR("考虑单位半径",GVT_S,GSem_Boolean,"计算半径范围时是否考虑单位的半径");
		GELEM_VAR_INIT( i_math::rangef,_rngRadius,i_math::rangef(0.0f,10.0f));
			GELEM_EDITVAR( "半径范围", GVT_Fx2,GSem_Range,"半径范围");
		GELEM_VAR_INIT( i_math::rangef,_rngFace,i_math::rangef(-180.0f,180.0f));
			GELEM_EDITVAR( "朝向范围", GVT_Fx2,GSem_Range,"朝向范围");
    END_GOBJ();    

public: //当作protected
	DEFINE_BVR(BgnLevelSkillTarget,_target);

	BOOL _bConsiderUnitRadius;

	i_math::rangef _rngRadius;
	i_math::rangef _rngFace;
};


struct LevelRecordSkill;
class CBgnThreat_CheckRange:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_CheckRange);

	CBgnThreat_CheckRange()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:
};

