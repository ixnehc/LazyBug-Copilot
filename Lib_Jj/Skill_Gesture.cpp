/********************************************************************
	created:	2016/09/02 
	author:		cxi
	
	purpose:	通用的Skill
*********************************************************************/
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "Skill_Gesture.h"

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

#include "Buff_PB.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_Gesture
BIND_SKILLPARAM(Skill_Gesture,SkillParam_Gesture);


void Skill_Gesture::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);
	_SetState(SkillState_Casting);

	BOOL bOk=FALSE;
	if (TRUE)
	{
		SkillParam_Gesture*param=_rec->GetParam<SkillParam_Gesture>();
		if (param)
		{
			if (param->idGesture!=RecordID_Invalid)
			{
				LevelRecordGesture *rec=GetLevel()->GetRecords()->GetGesture(param->idGesture);
				if (rec)
				{
					extern CLevelGesture *CreateGesture(CLevelObj *owner,LevelRecordGesture *recGesture,LevelSkillTarget &target);
					_ges=CreateGesture(_owner,rec,_target);
					if (_ges)
					{
						CUnit *unit=_owner->GetUnit();
						if (unit)
							unit->SetGesture(_ges);
						else
						{
							CUnit3D *unit3D=_owner->GetUnit3D();
							if (unit3D)
								unit3D->SetGesture(_ges);
						}

						_bFirstSync=TRUE;
						_AddSyncDataOp();
						bOk=TRUE;
					}
				}
			}

			if (param->idPathRes!=RecordID_Invalid)
			{
				CLevelResources *res=GetLevel()->GetResources();
				if (res)
				{
					LevelPathes *pathes=res->FindPathes(param->idPathRes);
					if (pathes)
						_events=&pathes->events;
				}
			}
		}
	}

	if (!bOk)
		_SetState(SkillState_Fail);
}

void Skill_Gesture::_CreateEoFromEvent(StringID nmEvent,i_math::xformf &xfmEvent,std::vector<GeneralSkillEoEntry> &entriesEo)
{
	CLevel *level=GetLevel();
	for (int i=0;i<entriesEo.size();i++)
	{
		if (!entriesEo[i].bEnable)
			continue;
		if (entriesEo[i].nmEvent!=nmEvent)
			continue;
		RecordID idEo=entriesEo[i].idEO;

		LevelRecordEo *rec=level->GetRecords()->GetEo(idEo);
		CLoEffectObj *eo=NULL;
		if (rec)
		{
			eo=(CLoEffectObj*)level->CreateObj(rec->param->GetEoClass());
			if (eo)
			{
				LevelOpLink link;
				link.id=level->GenOpLinkID();
				link.t=_tCasting;
				eo->PostCreate(_owner->GetPlayerID(),idEo,xfmEvent,NULL,1,LevelOSB(this),link);
				level->AddToActives(eo);
				SAFE_RELEASE(eo);
			}
		}
	}
}


void Skill_Gesture::_OnUpdate(AnimTick dt)
{
	SkillParam_Gesture*param=_rec->GetParam<SkillParam_Gesture>();

	_eventsFrame.clear();

	CLevel *level=GetLevel();
	if (!level)
		return;
	if (!_owner)
		return;

	if (_ges)
	{
		if (!_ges->IsAlive())
		{
			_Finish();
			return;
		}
	}


	if (_state==SkillState_Casting)
	{

		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		_AddSyncDataOp();

		if (_ges)
		{
			_ges->UpdateEvent(dt);
			i_math::xformf xfmEvent;
			while(1)
			{
				StringID nmEvent=_ges->FetchEvent(xfmEvent);
				if (nmEvent==StringID_Invalid)
					break;
				_eventsFrame.insert(nmEvent);

				if (_HandleEvent(nmEvent,xfmEvent))
					continue;//已处理

				_CreateEoFromEvent(nmEvent,xfmEvent,param->entriesEo);
			}
		}

		if (_events)
		{
			while(_iNextEvent<(*_events).size())
			{
				if ((*_events)[_iNextEvent].tEvent>_tCasting)
					break;

				StringID nmEvent=(*_events)[_iNextEvent].name;
				LevelPathesEvent *e=&(*_events)[_iNextEvent];
				_iNextEvent++;

				_eventsFrame.insert(nmEvent);

				i_math::xformf xfmEvent;

				if (TRUE)
				{
					i_math::matrix43f matBase;
					i_math::quatf rotBase;
					if (TRUE)
					{
						i_math::xformf xfm;
						xfm.pos=_owner->GetFramePos3D();

						i_math::vector3df euler;
						euler.x=LevelFaceToEuler(_owner->GetFrameFace());
						xfm.rot.fromEuler(euler);

						xfm.getMatrix(matBase);
						rotBase=xfm.rot;
					}

					i_math::vector3df pos=e->xfm.pos;
					pos*=_owner->GetModelScale();
					matBase.transformVect(pos,pos);

					xfmEvent.pos=pos;
					xfmEvent.rot=rotBase*e->xfm.rot;
				}

				_CreateEoFromEvent(nmEvent,xfmEvent,param->entriesEo);
			}
		}

	}
}

void Skill_Gesture::_OnBreak()
{
	_Finish();
}


void Skill_Gesture::_Finish()
{
	if (_ges)
		_ges->Finish();
	SAFE_RELEASE(_ges);
	_eventsFrame.clear();
	_SetState(SkillState_Finished);
}

BOOL Skill_Gesture::_WriteSyncData(CBitPacket *bp)
{
	if (!_ges)
		return FALSE;

	if (_bFirstSync)
	{
		_ges->WriteFirstSync(bp);
		_bFirstSync=FALSE;
		return TRUE;
	}
	else
		return _ges->WriteSync(bp);
}

static StringID GetSystemEvent_SwitchGround()
{
	static StringID nm=StringID_Invalid;
	if (nm==StringID_Invalid)
	{
		nm=StrLib_Get()->FindStr(0,"[Sys]SwitchGround","动画事件");
		assert(nm!=StringID_Invalid);
	}
	return nm;
}

BOOL Skill_Gesture::_HandleEvent(StringID nmEvent,i_math::xformf &xfmEvent)
{
	if (nmEvent==GetSystemEvent_SwitchGround())
	{
		CLevelObjMove *move=_owner->GetMove();
		if (move)
		{
			LevelPos3D pos3D=_owner->GetFramePos3D();
			LevelFace face=_owner->GetFrameFace();
			move->SwitchGround(pos3D.getXZ(),face,LevelTeleportID_Invalid);
		}
		
		return TRUE;
	}

	return FALSE;

}
