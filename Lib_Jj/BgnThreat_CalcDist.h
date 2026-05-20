#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpThreat_CalcDist:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_CalcDist);

	enum TargetType
	{
		Target_Threat,
		Target_Custom,

		Target_ForceDword=0xffffffff,
	};


	virtual const char *GetTypeName()	{		return "计算到Threat的距离";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Threat;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (varDist!=StringID_Invalid)
			FormatString(s,"计算从%s到%s的距离,保存在[%s]内",src.GetDesc(),target.GetDesc(),assist->GetStr(varDist));
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_CalcDist,1);
		GELEM_BGP_BASE();

		GELEM_OBJ(BgnLevelSkillTarget,src);GELEM_UID(2); 
			GELEM_EDITOBJ("源","源");

		GELEM_OBJ(BgnLevelSkillTarget,target);GELEM_UID(3); 
			GELEM_EDITOBJ("目标","目标");

		GELEM_BEHAVIORMEM_NUMBER(varDist,"[out]距离","距离变量"); GELEM_UID(4); 

    END_GOBJ();    

public: //当作protected

	BgnLevelSkillTarget src;
	BgnLevelSkillTarget target;

	StringID varDist;

};


struct LevelRecordSkill;
class CBgnThreat_CalcDist:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_CalcDist);

	CBgnThreat_CalcDist()
	{
	}



	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

