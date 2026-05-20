/********************************************************************
	created:	2016/09/02 
	author:		cxi
	
	purpose:	通用的Skill
*********************************************************************/
#include "stdh.h"

#include "commondefines/general_stl.h" 

#include "Skill_General.h"

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



////////////////////////////////////////////////////////////////////////
//CSkillGesture_GeneralEO
void CSkillGesture_Path::Create(KeySet *ksPos,KeySet *ksFace,AnimTick dur)
{
	_tCur=0.0f;

	assert(ksPos);
	assert(ksFace);
	_ksPos=ksPos;
	_ksFace=ksFace;
	_dur=dur;

	Key_2f k;
	if (_ksPos->CalcKey(ANIMTICK_FROM_SECOND(0.0f),&k))
		_posLast=_posCur=k.v;

	_bAlive=TRUE;
}


void CSkillGesture_Path::Update(CUnit *unit,float dt)
{
	if (_bFinished)
		return;
	if ((!_ksPos)||(!_ksFace))
		return;
	assert(_ksPos->GetKeyCount()>0);
	if (_ksPos->GetKeyCount()<=0)
		return;
	assert(_ksPos->GetKeyCount()==_ksFace->GetKeyCount());
	_tCur+=dt;

	if (TRUE)
	{
		Key_2f k;
		if (_ksPos->CalcKey(ANIMTICK_FROM_SECOND(_tCur),&k))
			unit->_pos=k.v;
	}

	if (TRUE)
	{
		Key_f k;
		if (KeySet_CalcAngleKey(_ksFace,&k,ANIMTICK_FROM_SECOND(_tCur)))
			unit->_face=k.v;
	}

	_posLast=_posCur;
	_posCur=unit->_pos;

	if (_tCur>=ANIMTICK_TO_SECOND(_dur))
		_bFinished=TRUE;
}

void CSkillGesture_Path::GetCurStep(i_math::line2df &step)
{
	step.start=_posLast;
	step.end=_posCur;
}




//////////////////////////////////////////////////////////////////////////
//CSkill_General
BIND_SKILLPARAM(Skill_General,SkillParam_General);

void Skill_General::CalcStartFace(CLevelObj *lo,LevelSkillTarget &target,SkillParam_General *param,LevelFace &face)
{
	extern LevelFace LevelUtil_CalcTargetFacing(LevelFace faceInitial,CLevelObj *lo,LevelSkillTarget &target,LevelSkillTargetFacingMode mode,float angleMaxAdjust);
	face=LevelUtil_CalcTargetFacing(lo->GetFrameFace(),lo,target,param->modeInitialFacing,param->angleMaxInitialFacingAdjust);
}


void Skill_General::CalcStartXfm(CLevelObj *lo,LevelSkillTarget &target,SkillParam_General *param,LevelXfm &xfm)
{
	xfm.pos=lo->GetFramePos();
	Skill_General::CalcStartFace(lo,target,param,xfm.face);
}


BOOL Skill_General::PreInitStartCheck(CLevelObj *owner,LevelRecordSkill *rec,LevelSkillTarget &target)
{
	SkillParam_General *param=rec->GetParam<SkillParam_General>();
	if (!param)
		return FALSE;

	LevelXfm xfmBase;
	CalcStartXfm(owner,target,param,xfmBase);

	extern BOOL LevelUtil_CheckPathValidity(CLevelObj *lo,RecordID idPath,LevelXfm &xfmBase);
	return LevelUtil_CheckPathValidity(owner,param->idPathRes,xfmBase);
}


void Skill_General::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);
	_SetState(SkillState_Casting);

	BOOL bOk=FALSE;
	if (TRUE)
	{
		SkillParam_General*param=_rec->GetParam<SkillParam_General>();
		if (param)
		{
			CUnit *unit=_owner->GetUnit();
			if (unit)
			{
				_ges=Class_New2(CSkillGesture_Path);

				LevelXfm xfm;
				CalcStartXfm(_owner,_target,param,xfm);

				extern BOOL LevelUtil_BuildPathKeyset(CLevel *level,KeySet &ksPos,KeySet &ksFace,AnimTick &dur,std::vector<LevelPathesEvent> *&events,LevelXfm &xfmBase,float scale,RecordID idPathRes);
				if (LevelUtil_BuildPathKeyset(GetLevel(),_ksPos,_ksFace,_dur,_events,xfm,_owner->GetModelScale(),param->idPathRes))
				{
					if (param->dur>0)
						_dur=param->dur;

					LevelOp_Path*op=(LevelOp_Path*)NewOp<LevelOp_Path>(LevelOpLink());
					op->ksPos.CopyFrom(_ksPos);
					op->ksFace.CopyFrom(_ksFace);
					op->dur=_dur;
					_owner->AddOp(op);

					_dur+=ANIMTICK_FROM_SECOND(0.2f);
					_ges->Create(&_ksPos,&_ksFace,_dur);
					_ges->AddRef();
					unit->SetGesture(_ges);
					bOk=TRUE;
				}

				_xfmBase=xfm;
			}
		}
	}

	if (!bOk)
		_SetState(SkillState_Fail);

}

void Skill_General::_CalcEventXfm(LevelPathesEvent &e,LevelXfm &xfmEvent)
{
	i_math::matrix43f matBase;
	i_math::quatf rotBase;
	if (TRUE)
	{
		i_math::xformf xfm;
		xfm.pos.x=_xfmBase.pos.x;
		xfm.pos.y=0.0f;
		xfm.pos.z=_xfmBase.pos.y;

		i_math::vector3df euler;
		euler.x=LevelFaceToEuler(_xfmBase.face);
		xfm.rot.fromEuler(euler);

		xfm.getMatrix(matBase);
		rotBase=xfm.rot;
	}

	i_math::vector3df pos=e.xfm.pos;
	pos*=_owner->GetModelScale();
	matBase.transformVect(pos,pos);

	xfmEvent.pos=pos.getXZ();

	i_math::vector3df euler;
	(rotBase*e.xfm.rot).toEuler(euler);

	xfmEvent.face=LevelFaceFromEuler(euler.x);

}

void Skill_General::_CalcEventXfm(LevelPathesEvent &e,i_math::xformf &xfmEvent)
{
	i_math::matrix43f matBase;
	i_math::quatf rotBase;
	if (TRUE)
	{
		i_math::xformf xfm;
		xfm.pos.x=_xfmBase.pos.x;
		xfm.pos.y=0.0f;
		xfm.pos.z=_xfmBase.pos.y;

		i_math::vector3df euler;
		euler.x=LevelFaceToEuler(_xfmBase.face);
		xfm.rot.fromEuler(euler);

		xfm.getMatrix(matBase);
		rotBase=xfm.rot;
	}

	i_math::vector3df pos=e.xfm.pos;
	pos*=_owner->GetModelScale();
	matBase.transformVect(pos,pos);

	xfmEvent.pos=pos;
	i_math::quatf rot=e.xfm.rot;
	rot.normalize();
	xfmEvent.rot=rot*rotBase;

	//计算精确的高度
	if (TRUE)
	{
		Key_2f k;
		if (_ksPos.CalcKey(e.tEvent,&k))
		{
			i_math::vector3df pos;
			pos.setXZ(k.v);//注意这个位置已经根据ModelScale缩放过了

			pos=LevelUtil_GetGroundHeight(GetLevel(),pos.x,pos.z,TRUE);
			xfmEvent.pos.y+=pos.y;
		}
	}
}



void Skill_General::_OnUpdate(AnimTick dt)
{
	_eventsFrame.clear();

	CLevel *level=GetLevel();
	if (!level)
		return;
	if (!_owner)
		return;

	if (_state==SkillState_Casting)
	{
		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);
		SkillParam_General*param=_rec->GetParam<SkillParam_General>();

		if (_ges)
		{
			i_math::line2df step;
			_ges->GetCurStep(step);

			CLevelObjMap *mpObj=level->GetObjMap();

			float radiusSrc=_owner->GetRadius_();
			i_math::vector2df vRay=step.end-step.start;
			float dist=vRay.getLength();
			if (dist>=0.001f)
			{
				i_math::vector2df vRayDir=vRay/dist;

				LevelUtilDetectParam param;
				param.loSrc=_owner;
				param.pos=(step.start+step.end)*0.5f;//中点
				param.rangeMin=0.0f;
				param.rangeMax=(float)step.getLength()/2.0f+radiusSrc;
				LevelDetectTargetFlag flag;
				flag=(LevelDetectTargetFlag)(LevelDetectTarget_Enemy|LevelDetectTarget_Unit|LevelDetectTarget_Player|LevelDetectTarget_Ground);
				param.flags=&flag;
				param.nFlags=1;
				LevelObjRequire require=LevelObjRequire_Attackable;
				param.requires=&require;
				param.nRequires=1;

				DWORD c;
				extern CLevelObj **LevelUtil_Detect(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,DWORD &c);
				CLevelObj **buf=LevelUtil_Detect(param,NULL,c);

				extern float intersectSphereBySphere(i_math::vector2df& vSrc,float rSrc,i_math::vector2df& vRay, i_math::vector2df& vTarget, float rTarget);

				AnimTick t=ANIMTICK_FROM_SECOND(_ges->GetCurT());

				for (int i=0;i<c;i++)
				{
					CLevelObj *loTarget=buf[i];
					if (TRUE)
					{
						CLevelObjPauser *pauser=loTarget->GetPauser();
						if (pauser)
						{
							if (pauser->IsWorking())
							{
								if (!pauser->IsMovePaused())
									continue;//这个单位还没有移到该移的位置
							}
						}
					}
					float radiusTarget=loTarget->GetRadius_();

					LevelPos posTarget=loTarget->GetFramePos();
					if (intersectSphereBySphere(step.start,radiusSrc,vRay,posTarget,radiusTarget)<dist)
					{
						//有碰撞
						LevelPos posProj;
						step.getProjectionPoint(posTarget,posProj);
						float distProj;
						distProj=posTarget.getDistanceFrom(posProj);

						float distKB=radiusTarget+radiusSrc-distProj;
						distKB+=0.3f;//再稍稍推远一点
						LevelPos dirKB;
						if (distProj>0.001f)
							dirKB=(posTarget-posProj)/distProj;
						else
						{//在直线上,推向一侧
							dirKB.y=-vRayDir.x;
							dirKB.x=vRayDir.y;
						}


						BuffArg_PB arg;
						arg.posTarget=loTarget->GetFramePos()+dirKB*distKB;
						arg.face=atan2f(-dirKB.y,-dirKB.x);

						LevelOpLink link;
						link.id=GetLevel()->GenOpLinkID();
						link.t=t;

						level->GetDecider()->MakeBuff(LevelOSB(this),loTarget,level->GetRecords()->GetGlobal()->idDefBuff_PB,0,&arg,link);
					}
				}
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

				for (int i=0;i<param->entriesEo.size();i++)
				{
					if (!param->entriesEo[i].bEnable)
						continue;
					if (param->entriesEo[i].nmEvent!=nmEvent)
						continue;
					RecordID idEo=param->entriesEo[i].idEO;

					LevelRecordEo *rec=level->GetRecords()->GetEo(idEo);
					CLoEffectObj *eo=NULL;
					if (rec)
					{
						eo=(CLoEffectObj*)level->CreateObj(rec->param->GetEoClass());
						if (eo)
						{
							i_math::xformf xfmEvent;
							_CalcEventXfm(*e,xfmEvent);

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
		}

// 
// 
// 		//释放Deal
// 		if (_nSpawned<param->nDeal)
// 		{
// 			if (param->nDeal>0)
// 			{
// 				_tCur+=dt;
// 				AnimTick durPerEO=param->dur/param->nDeal;
// 				DWORD nToSpawn;
// 				if (durPerEO<=0)
// 					nToSpawn=param->nDeal;
// 				else
// 				{
// 					nToSpawn=_tCur/durPerEO+1;
// 					if (nToSpawn>param->nDeal)
// 						nToSpawn=param->nDeal;
// 				}
// 
// 				if (_ges)
// 				{
// 					if (!_ges->IsAlive())
// 						nToSpawn=param->nDeal;
// 				}
// 
// 
// 				while(nToSpawn>_nSpawned)
// 				{
// 					AnimTick t=_nSpawned*durPerEO;
// 
// 					Key_2f k;
// 					_ksPath.CalcKey(t,&k);
// 					LevelPos pos=k.v;
// 
// 					//在pos的位置释放一个Deal
// 					if (_rec->deal)
// 					{
// 						DealArg arg;
// 						arg.grd=_grd;
// 						arg.dir.set(0,0);
// 						arg.idLink=LevelOpLinkID_Invalid;
// 						arg.iSerial=0;
// 						_MakeDeals(pos,arg);
// 					}
// 					_nSpawned++;
// 				}
// 			}
// 		}


		if (_ges)
		{
			if (!_ges->IsAlive())
				_Finish();
		}
	}
}

void Skill_General::_OnBreak()
{
	_Finish();
}


void Skill_General::_Finish()
{
	if (_ges)
		_ges->Stop();
	SAFE_RELEASE(_ges);
	_ksPos.Clean();
	_ksFace.Clean();
	_eventsFrame.clear();
	_SetState(SkillState_Finished);
}

AnimTick Skill_General::GetCastingEventTime(StringID nmEvent)
{
	if (_events)
	{
		for (int i=0;i<(*_events).size();i++)
		{
			AnimEvent *e=&(*_events)[i];
			if (e->name==nmEvent)
				return e->tEvent;
		}
	}
	return ANIMTICK_INFINITE;
}
