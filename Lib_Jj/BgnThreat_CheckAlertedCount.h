#pragma once

#include "math/range.h"

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpThreat_CheckAlertedCount:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_CheckAlertedCount);

	virtual const char *GetTypeName()	{		return "检测Threat的Alerted个数";	}
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
		if ((_rng.low<=0)&&(_rng.hi>=1000))
			s="Threat的Alerted个数为任意值时";
		else
		{
			if (_rng.low<=0)
				FormatString(s,"Threat的Alerted个数小于等于%d时",_rng.hi);
			else
			{
				if (_rng.hi>=1000)
					FormatString(s,"Threat的Alerted个数大于等于%d时",_rng.low);
				else
					FormatString(s,"Threat的Alerted个数介于[%d,%d]时",_rng.low,_rng.hi);
			}
		}

	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_CheckAlertedCount,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( i_math::rangei,_rng,i_math::rangei(0,10000));
			GELEM_EDITVAR( "个数范围", GVT_Sx2,GSem_Range,"个数范围");
    END_GOBJ();    

public: //当作protected

	i_math::rangei _rng;
};


struct LevelRecordSkill;
class CBgnThreat_CheckAlertedCount:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_CheckAlertedCount);

	CBgnThreat_CheckAlertedCount()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:
};

