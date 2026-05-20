#pragma once

#include "LevelSkill.h"

#include "LevelGesture.h"

#include "anim/KeySet.h"

#include "Skill_General.h"


struct SkillParam_PushSlideway:public SkillParam_General
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_PushSlideway);

	BEGIN_GOBJ_PURE(SkillParam_PushSlideway,1);

		GELEM_VAR_INIT(RecordID,idPathRes,RecordID_Invalid);
			GELEM_EDITVAR("移动路径资源(Push)",GVT_U,GSem(GSem_RecordID,"resources"),"移动路径资源");
		GELEM_VAR_INIT(RecordID,idPathResAbortL,RecordID_Invalid);
			GELEM_EDITVAR("移动路径资源(PushAbortL)",GVT_U,GSem(GSem_RecordID,"resources"),"移动路径资源");
		GELEM_VAR_INIT(RecordID,idPathResAbortR,RecordID_Invalid);
			GELEM_EDITVAR("移动路径资源(PushAbortR)",GVT_U,GSem(GSem_RecordID,"resources"),"移动路径资源");
		GELEM_VAR_INIT(float,offsetPusher,0.5f);
			GELEM_EDITVAR("Pusher的(向后)偏移距离",GVT_F,GSem(GSem_Float,"0.0,10.0,0.01"),"Pusher的(向后)偏移距离");
	END_GOBJ();

	RecordID idPathRes;
	RecordID idPathResAbortL;
	RecordID idPathResAbortR;

	float offsetPusher;
};


struct LevelPathesEvent;
class Skill_PushSlideway:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_PushSlideway,37);

	Skill_PushSlideway()
	{
		_tCasting=0;
		_spConsumed=0.0f;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_FixPosAndObj);
	}
	virtual CastMoving GetCastMoving()	{		return CastMoving_Control;	}
	virtual AnimTick GetCastingTime()	{		return _tCasting;	}//返回经过IAS修正的casting time


	virtual void NotifyCasted() override;

	virtual BOOL CanStopCast(AnimTick tStop)	{		return TRUE;	}//信任client
	virtual void StopCast(AnimTick tStop) override;

protected:
	virtual void _OnStart();
	virtual void _OnBreak();
	virtual void _OnUpdate(AnimTick dt);

	virtual void _OnFinish()	{		_Finish();	}



	void _Finish();

	AnimTick _tCasting;

	AnimTick _dur;

	float _spConsumed;

};

