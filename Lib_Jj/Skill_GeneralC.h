#pragma once

#include "LevelSkill.h"

#include "LevelGesture.h"

#include "anim/KeySet.h"

#include "Skill_General.h"


struct SkillParam_GeneralC:public SkillParam_General
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_GeneralC);

	BEGIN_GOBJ_PURE(SkillParam_GeneralC,1);DERIVE_GOBJ(SkillParam_GeneralC,SkillParam_General)

	END_GOBJ();
};

class CSkillGeneralEvents
{
public:
	CSkillGeneralEvents()
	{
		Zero();
	}
	void Zero()
	{
		_tEventsStart=0;
		_iNextEvent=0;
		_events=NULL;
		_entriesEo=NULL;
		_entriesOp=NULL;
		_bAbortUpdate=FALSE;
	}

	void Init(std::vector<GeneralSkillEoEntry> *entriesEo,std::vector<GeneralSkillOpEntry> * entriesOp);
	void Clear();
	void SetEvents(std::vector<LevelPathesEvent> *events,AnimTick tEventsStart);
	void Update(CLevelSkill *skillOwner,AnimTick tCasting);

	BOOL CheckCastingEvent(StringID nmEvent)	{		return _eventsFrame.find(nmEvent)!=_eventsFrame.end();	}
	AnimTick GetCastingEventTime(StringID nmEvent);
	BOOL CheckWindow(StringID nmOpen,StringID nmClose,AnimTick t);

	void ClearFrameHistory()
	{
		_eventsFrame.clear();
	}

	static void CreateEo(RecordID idEo,CLevelSkill *skill,i_math::xformf &xfm,AnimEventZone *eSrc,AnimTick t);

protected:

	void _CalcEventXfm(CLevelObj *owner,LevelPathesEvent &e,i_math::xformf &xfmEvent);

	std::vector<GeneralSkillEoEntry> * _entriesEo;
	std::vector<GeneralSkillOpEntry> * _entriesOp;
	std::unordered_set<StringID> _eventsFrame;
	std::vector<LevelPathesEvent> *_events;
	AnimTick _tEventsStart;
	int _iNextEvent;//下一个事件

	BOOL _bAbortUpdate;

};


struct LevelPathesEvent;
class Skill_GeneralC:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_GeneralC,33);

	Skill_GeneralC()
	{
		_tCasting=0;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_Pos)|(1<<LevelSkillTarget::Target_Aim)|(1<<LevelSkillTarget::Target_None)|(1<<LevelSkillTarget::Target_DefObj)
			|(1<<LevelSkillTarget::Target_FixPosAndObj);
	}
	virtual CastMoving GetCastMoving()	{		return CastMoving_Control;	}
	virtual AnimTick GetCastingTime()	{		return _tCasting;	}//返回经过IAS修正的casting time

	virtual BOOL CheckCastingEvent(StringID nmEvent)	{		return _events.CheckCastingEvent(nmEvent);	}
	virtual AnimTick GetCastingEventTime(StringID nmEvent);

	virtual BOOL CanStopCast(AnimTick tStop)	{		return TRUE;	}//信任client
	virtual void StopCast(AnimTick tStop) override;

	void OnOp(GeneralSkillOpEntry &entryOp);

protected:
	virtual void _OnStart();
	virtual void _OnBreak();
	virtual void _OnUpdate(AnimTick dt);

	virtual void _OnFinish()	{		_Finish();	}

	void _CalcEventXfm(LevelPathesEvent &e,i_math::xformf &xfmEvent);

	void _PumpEvents(AnimTick tCasting);


	void _Finish();

	AnimTick _tCasting;

	AnimTick _dur;

	CSkillGeneralEvents _events;
};

