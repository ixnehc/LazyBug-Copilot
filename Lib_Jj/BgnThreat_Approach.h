#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpThreat_Approach:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_Approach);

	virtual const char *GetTypeName()	{		return "逼近Threat";	}
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
		std::string nm=assist->GetSkillName(idSkill);
		if (!nm.empty())
		{
			FormatString(s,"[ %s ]技能逼近",nm.c_str());
			if (dur>0)
				AppendFmtString(s,",最大尝试%.2f秒",ANIMTICK_TO_SECOND(dur));
			if (speed>0)
				AppendFmtString(s,",移动速度%.2f米/秒",speed);
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_Approach,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("使用技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
			GELEM_BVR();
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("最大尝试时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"最大尝试时间,0表示永远");
		GELEM_VAR_INIT(float,speed,0.0f);
			GELEM_EDITVAR("移动速度",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"移动速度,0表示使用默认速度");
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(RecordID,idSkill);
	AnimTick dur;
	float speed;
};


struct LevelRecordSkill;
struct AttrNodeBase;
class CBgnThreat_Approach:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_Approach);

	CBgnThreat_Approach()
	{
		_target=NULL;
		_verCast=0xffffffff;
		_tStart=0;
		_attrnode=NULL;
	}



	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:
	void _Start(CLevelObj *target);
	void _Stop();

	CLevelObj *_target;

	AnimTick _tStart;
	DWORD _verCast;

	AttrNodeBase *_attrnode;

};

