#pragma once

#include "LevelSkill.h"

#include "LevelGesture.h"

#include "anim/KeySet.h"

#include "Skill_General.h"


struct SkillParam_Gesture:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_Gesture);

	BEGIN_GOBJ_PURE(SkillParam_Gesture,1);

		GELEM_VAR_INIT(RecordID,idPathRes,RecordID_Invalid);
			GELEM_EDITVAR("移动路径资源",GVT_U,GSem(GSem_RecordID,"resources"),"移动路径资源");
		GELEM_VAR_INIT(RecordID,idGesture,RecordID_Invalid);
			GELEM_EDITVAR("Gesture",GVT_U,GSem(GSem_RecordID,"gestures"),"Gesture");
		GELEM_OBJVECTOR(GeneralSkillEoEntry,entriesEo);
			GELEM_EDITOBJ("EO参数","EO参数");
	END_GOBJ();

	RecordID idPathRes;
	RecordID idGesture;
	std::vector<GeneralSkillEoEntry> entriesEo;

};


struct LevelPathesEvent;
class Skill_Gesture:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_Gesture,29);

	Skill_Gesture()
	{
		_ges=NULL;
		_tCasting=0;
		_bFirstSync=TRUE;
		_iNextEvent=0;
		_events=0;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_Aim)|
					(1<<LevelSkillTarget::Target_None)|
					(1<<LevelSkillTarget::Target_DefObj)|
					(1<<LevelSkillTarget::Target_FixPosAndObj);
	}
	virtual CastMoving GetCastMoving()	{		return CastMoving_Control;	}
	virtual AnimTick GetCastingTime()	{		return _tCasting;	}//返回经过IAS修正的casting time

	virtual BOOL CheckCastingEvent(StringID nmEvent)	{		return _eventsFrame.find(nmEvent)!=_eventsFrame.end();	}

protected:
	virtual void _OnStart();
	virtual void _OnBreak();
	virtual void _OnUpdate(AnimTick dt);

	virtual void _OnFinish()	{		_Finish();	}
	virtual BOOL _WriteSyncData(CBitPacket *bp);

	BOOL _HandleEvent(StringID nmEvent,i_math::xformf &xfmEvent);//返回是否处理了

	void _CreateEoFromEvent(StringID nmEvent,i_math::xformf &xfmEvent,std::vector<GeneralSkillEoEntry> &entriesEo);


	void _Finish();

	AnimTick _tCasting;

	std::unordered_set<StringID> _eventsFrame;
	CLevelGesture *_ges;
	BOOL _bFirstSync;

	std::vector<LevelPathesEvent> *_events;
	int _iNextEvent;//下一个事件

};

