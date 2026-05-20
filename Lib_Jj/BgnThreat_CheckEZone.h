#pragma once

#include "math/range.h"

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpThreat_CheckEZone:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_CheckEZone);

	virtual const char *GetTypeName()	{		return "检测Threat(事件区域)";	}
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
		s="n/a";
		if (_nmEvent!=StringID_Invalid)
		{
			if (_bCheckTimeRange)
			{
				FormatString(s,"检测%s是否\n在当前技能阶段的\"%s\"事件区域及时间范围内",
					BgnLevelSkillTarget::GetDesc(BVR_ARG(_target)),
					assist->GetStr(_nmEvent));
			}
			else
			{
				if (_nmSkillStage==StringID_Invalid)
				{
					FormatString(s,"检测%s是否\n在当前技能阶段的\"%s\"事件区域内",
						BgnLevelSkillTarget::GetDesc(BVR_ARG(_target)),
						assist->GetStr(_nmEvent));
				}
				else
				{
					if (_idSkill==RecordID_Invalid)
					{
						FormatString(s,"检测%s是否\n在当前技能的[%s]阶段的\"%s\"事件区域内",
							BgnLevelSkillTarget::GetDesc(BVR_ARG(_target)),assist->GetStr(_nmSkillStage),assist->GetStr(_nmEvent));
					}
					else
					{
						FormatString(s,"检测%s是否\n在[%s]技能的[%s]阶段的\"%s\"事件区域内",
							BgnLevelSkillTarget::GetDesc(BVR_ARG(_target)),
							assist->GetSkillName(_idSkill),assist->GetStr(_nmSkillStage),assist->GetStr(_nmEvent));
					}
				}
			}
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgpThreat_CheckEZone,485,1);
		GELEM_BGP_BASE();

		GELEM_OBJ(BgnLevelSkillTarget,_target);GELEM_UID(1); 
			GELEM_EDITOBJ("目标","目标");
			GELEM_BVR();

		GELEM_VAR_INIT(StringID,_nmEvent,StringID_Invalid);GELEM_UID(2);
			GELEM_EDITVAR("事件",GVT_U,GSem(GSem_StringID,"动画事件"),"侦听的动画事件名称")

		GELEM_VAR_INIT(BOOL ,_bCheckTimeRange,FALSE);GELEM_UID(3);
		GELEM_EDITVAR("是否检测时间范围",GVT_S,GSem(GSem_Interger,
			"检测时间范围:1"		"|技能&技能阶段名称,"
			"不检测时间范围:0"	""
			),"是否检测时间范围");
		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_U,GSem(GSem_RecordID,"skills"),"Skill");
		GELEM_VAR_INIT(StringID,_nmSkillStage,StringID_Invalid);GELEM_UID(2);
			GELEM_EDITVAR("技能阶段名称",GVT_U,GSem(GSem_StringID,"技能阶段"),"技能阶段名称");

    END_GOBJ();    

public: //当作protected
	DEFINE_BVR(BgnLevelSkillTarget,_target);
	RecordID _idSkill;
	StringID _nmSkillStage;
	StringID _nmEvent;
	BOOL _bCheckTimeRange;


};


struct LevelRecordSkill;
class CBgnThreat_CheckEZone:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_CheckEZone);

	CBgnThreat_CheckEZone()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:
};

