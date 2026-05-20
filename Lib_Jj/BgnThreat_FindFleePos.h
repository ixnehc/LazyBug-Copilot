#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpThreat_FindFleePos:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_FindFleePos);

	virtual const char *GetTypeName()	{		return "寻找逃离Threat的位置";	}
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
		if (_varPos!=StringID_Invalid)
		{
			FormatString(s,"找到逃离Theat的位置,存放在[%s]",assist->GetStr(_varPos));
			AppendFmtString(s,"\n逃离距离%s米,",GetBVRDesc_Float(BVR_ARG(_dist),assist));
			AppendFmtString(s,"最小需要逃离%s米",GetBVRDesc_Float(BVR_ARG(_distMin),assist));
			if (_radiusToOwner>0.0f)
			{
				AppendFmtString(s,"\n逃离位置要求保持在离owner%s米的范围内",
					GetBVRDesc_Float(BVR_ARG(_radiusToOwner),assist)
					);
			}

		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_FindFleePos,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_POS(_varPos,"[out]位置变量","找到的位置存放在哪里")
		GELEM_BEHAVIORMEM_INTERGER(_varSignAvoid,"[inout]回避方向变量","回避方向变量")
		GELEM_VAR_INIT(float,_dist,5.0f);
			GELEM_EDITVAR("逃离距离",GVT_F,GSem(GSem_Float,"0,200,0.1"),"逃离距离");
			GELEM_BVR();
		GELEM_VAR_INIT(float,_distMin,1.0f);
			GELEM_EDITVAR("最小逃离距离",GVT_F,GSem(GSem_Float,"0,200,0.1"),"至少要逃离多远的距离");
			GELEM_BVR();
		GELEM_VAR_INIT(float,_radiusToOwner,0.0f);
			GELEM_EDITVAR("离owner的距离",GVT_F,GSem(GSem_Float,"0,200,0.1"),"逃跑位置限制在owner的多大半径范围内,0表示不需要考虑owner的位置");
			GELEM_BVR();
    END_GOBJ();    

public: //当作protected

	StringID _varPos;
	StringID _varSignAvoid;
	DEFINE_BVR(float,_dist);
	DEFINE_BVR(float,_distMin);
	DEFINE_BVR(float,_radiusToOwner);
};


struct LevelRecordSkill;
class CBgnThreat_FindFleePos:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_FindFleePos);

	CBgnThreat_FindFleePos()
	{
	}



	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Destroy();

protected:


};

