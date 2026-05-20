/********************************************************************
	created:	2016/09/02 
	author:		cxi
	
	purpose:	通用的Skill
*********************************************************************/
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "Skill_GeneralC.h"
#include "Skill_GeneralS.h"
#include "Skill_GeneralAdvS.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LoEffectObj.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "LevelDecider.h"
#include "LevelRecords.h"
#include "LevelRecordGlobal.h"

#include "LevelResources.h"

#include "LevelObjMove.h"
#include "LevelBlocking.h"

#include "Buff_PB.h"

#include "timer/timer.h"


//////////////////////////////////////////////////////////////////////////
//CSkillGeneralEvents
void CSkillGeneralEvents::Init(std::vector<GeneralSkillEoEntry> *entriesEo,
							   std::vector<GeneralSkillOpEntry> * entriesOp)
{
	_entriesEo=entriesEo;
	_entriesOp=entriesOp;
	_iNextEvent=0;
}

void CSkillGeneralEvents::SetEvents(std::vector<LevelPathesEvent> *events,AnimTick tEventsStart)
{
	_events=events;
	_tEventsStart=tEventsStart;
	_iNextEvent=0;

	_bAbortUpdate=TRUE;

}


void CSkillGeneralEvents::Clear()
{
	_eventsFrame.clear();
	Zero();
}

void CSkillGeneralEvents::CreateEo(RecordID idEo,CLevelSkill *skill,i_math::xformf &xfm,AnimEventZone *eSrc,AnimTick t)
{
	if (!skill)
		return;
	CLevelObj *owner=skill->GetOwner();
	if (!owner)
		return;

	CLevel *level=skill->GetLevel();

	LevelRecordEo *rec=level->GetRecords()->GetEo(idEo);
	CLoEffectObj *eo=NULL;
	if (rec)
	{
		eo=(CLoEffectObj*)level->CreateObj(rec->param->GetEoClass());
		if (eo)
		{
			LevelOpLink link;
			link.id=level->GenOpLinkID();
			link.t=t;
			eo->PostCreate(owner->GetPlayerID(),idEo,xfm,eSrc,1,LevelOSB(skill),link);
			level->AddToActives(eo);
			SAFE_RELEASE(eo);
		}
	}

}


void CSkillGeneralEvents::Update(CLevelSkill *skillOwner,AnimTick tCasting0)
{
	_bAbortUpdate=FALSE;
	
	if (!skillOwner)
		return;

	CLevelObj *owner=skillOwner->GetOwner();
	if (!owner)
		return;

	CLevel *level=owner->GetLevel();
	if (!level)
		return;

	if (_events)
	{
		AnimTick tCasting=ANIMTICK_SAFE_MINUS(tCasting0,_tEventsStart);
		while(_iNextEvent<(*_events).size())
		{
			if ((*_events)[_iNextEvent].tEvent>tCasting)
				break;

			StringID nmEvent=(*_events)[_iNextEvent].name;
			LevelPathesEvent *e=&(*_events)[_iNextEvent];
			_iNextEvent++;

			_eventsFrame.insert(nmEvent);

			if (_entriesEo)
			{
				for (int i=0;i<_entriesEo->size();i++)
				{
					if (!(*_entriesEo)[i].bEnable)
						continue;
					if ((*_entriesEo)[i].nmEvent!=nmEvent)
						continue;
					if ((*_entriesEo)[i].nmChecker!=StringID_Invalid)
					{
						CLevelBehavior *behavior=owner->GetBehaviorAI();
						if (behavior)
						{
							if (A_Fail==behavior->StartRelay((*_entriesEo)[i].nmChecker))
								continue;
						}
					}
					RecordID idEo=(*_entriesEo)[i].idEO;
					i_math::xformf xfmEvent;
					_CalcEventXfm(owner,*e,xfmEvent);

					extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
					if ((*_entriesEo)[i].bGround)
						xfmEvent.pos=LevelUtil_GetGroundHeight(level,xfmEvent.pos.x,xfmEvent.pos.z,TRUE);

					CreateEo(idEo,skillOwner,xfmEvent,&e->zone,tCasting0);
				}
			}

			if (_entriesOp)
			{
				for (int i=0;i<(*_entriesOp).size();i++)
				{
					if (!(*_entriesOp)[i].bEnable)
						continue;
					if ((*_entriesOp)[i].nmEvent!=nmEvent)
						continue;

					if (IsClass2(skillOwner,Skill_GeneralS))
					{
						((Skill_GeneralS*)skillOwner)->OnOp((*_entriesOp)[i]);
					}
					if (IsClass2(skillOwner,Skill_GeneralC))
					{
						((Skill_GeneralC*)skillOwner)->OnOp((*_entriesOp)[i]);
					}
				}
			}

			if (IsClass2(skillOwner,Skill_GeneralAdvS))
			{
				((Skill_GeneralAdvS*)skillOwner)->OnEvent(nmEvent,e->zone.IsValid()?&e->zone:NULL,e->tEvent);
			}

			if (_bAbortUpdate)
				break;
		}
	}
}

void CSkillGeneralEvents::_CalcEventXfm(CLevelObj *owner,LevelPathesEvent &e,i_math::xformf &xfmEvent)
{
	i_math::matrix43f matBase;
	i_math::quatf rotBase;
	if (TRUE)
	{
		LevelPos3D pos=owner->GetFramePos3D();		
		LevelFace face=owner->GetFrameFace();
		i_math::xformf xfm;
		xfm.pos=pos;

		i_math::vector3df euler;
		euler.x=LevelFaceToEuler(face);
		xfm.rot.fromEuler(euler);

		xfm.getMatrix(matBase);
		rotBase=xfm.rot;
	}

	i_math::vector3df pos=e.xfm.pos;
	pos*=owner->GetModelScale();
	matBase.transformVect(pos,pos);

	xfmEvent.pos=pos;
	i_math::quatf rot=e.xfm.rot;
	rot.normalize();
	xfmEvent.rot=rot*rotBase;
}
 
AnimTick CSkillGeneralEvents::GetCastingEventTime(StringID nmEvent)
{
	if (_events)
	{
		for (int i=0;i<(*_events).size();i++)
		{
			AnimEvent *e=&(*_events)[i];
			if (e->name==nmEvent)
				return e->tEvent+_tEventsStart;
		}
	}
	return ANIMTICK_INFINITE;
}

BOOL CSkillGeneralEvents::CheckWindow(StringID nmOpen,StringID nmClose,AnimTick t)
{
	if (!_events)
		return FALSE;

	BOOL bOpenFound=FALSE;
	int i;
	for (i=0;i<(*_events).size();i++)
	{
		AnimEvent *e=&(*_events)[i];
		if (e->name==nmOpen)
		{
			if (t>=e->tEvent+_tEventsStart)
			{
				bOpenFound=TRUE;
				break;
			}
			else
				return FALSE;
		}
	}

	if (!bOpenFound)
		return FALSE;
	if (nmClose==StringID_Invalid)
		return TRUE;

	for (;i<(*_events).size();i++)
	{
		AnimEvent *e=&(*_events)[i];
		if (e->tEvent+_tEventsStart>=t)
			break;
		if (e->name==nmClose)
			return FALSE;
	}
	return TRUE;
}




//////////////////////////////////////////////////////////////////////////
//CSkill_General
BIND_SKILLPARAM(Skill_GeneralC,SkillParam_GeneralC);


void Skill_GeneralC::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);
	_SetState(SkillState_Casting);

	_dur=0;

	BOOL bOk=FALSE;
	if (TRUE)
	{
		SkillParam_GeneralC*param=_rec->GetParam<SkillParam_GeneralC>();
		if (param)
		{
			CLevelResources *res=GetLevel()->GetResources();
			if (res)
			{
				LevelPathes *pathes=res->FindPathes(param->idPathRes);
				if (pathes)
				{
					_events.Init(&param->entriesEo,&param->entriesOp);
					_events.SetEvents(&pathes->events,_tCasting);
					if (pathes->def)
						_dur=pathes->def->dur;
					bOk=TRUE;
				}
			}

			if (param->dur>0)
				_dur=param->dur;
		}

	}

	if (!bOk)
		_SetState(SkillState_Fail);

//	GetLevel()->GetDbgDraw().DrawCircle(_owner->GetFramePos(),0.06f,RGB(0,0,255),5.f);

}



void Skill_GeneralC::_OnUpdate(AnimTick dt)
{
	if (_state==SkillState_Casting)
	{
		CLevel *level=GetLevel();
//		level->GetDbgDraw().DrawCircle(_owner->GetFramePos(),0.08f,RGB(0,255,0),5.0f);

		_events.ClearFrameHistory();

		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		_events.Update(this,_tCasting);

		if (_tCasting>_dur)
			_Finish();
	}
}

void Skill_GeneralC::_OnBreak()
{
	_Finish();
}


void Skill_GeneralC::_Finish()
{
	_events.Clear();

	_SetState(SkillState_Finished);
}

AnimTick Skill_GeneralC::GetCastingEventTime(StringID nmEvent)
{
	return _events.GetCastingEventTime(nmEvent);
}

void Skill_GeneralC::StopCast(AnimTick tCasting)
{
	_events.Update(this,tCasting);

	_Finish();
}

void Skill_GeneralC::OnOp(GeneralSkillOpEntry &entryOp)
{
	switch(entryOp.op)
	{
		case GeneralSkillOpEntry::Op_OpenBlocking:
		{
			CLevelBlocking *blocking=_owner->GetBlocking();
			if (blocking)
				blocking->Activate();
			break;
		}
		case GeneralSkillOpEntry::Op_CloseBlocking:
		{
			CLevelBlocking *blocking=_owner->GetBlocking();
			if (blocking)
				blocking->Deactiveate();
			break;
		}
	}
}

