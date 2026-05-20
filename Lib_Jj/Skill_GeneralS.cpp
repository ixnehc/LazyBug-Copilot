/********************************************************************
	created:	2016/09/02 
	author:		cxi
	
	purpose:	通用的Skill
*********************************************************************/
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "Skill_GeneralS.h"

#include "LevelRecordSkill.h"
#include "LevelRecordUnit.h"

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
//CSkillGesture_PathS
void CSkillGesture_PathS::Create(CLevel *level,BOOL bAllowFlying)
{
	_level=level;
	_bAllowFlying=bAllowFlying;
}

void CSkillGesture_PathS::ResetLoc(LevelPos &pos,float ht,float face)
{
	_pos=pos;
	_face=face;
	_ht=ht;

	if (_bAllowFlying)
	{
		_pos3D=LevelUtil_GetWalkableGroundHeight(_level,_pos.x,_pos.y,TRUE);
		_pos3D.setXZ(_pos);
		_pos3D.y+=_ht;

		_vel.setZero();
	}
}

void CSkillGesture_PathS::UpdateLoc(LevelPos &pos,float ht,float face,AnimTick dt)
{
	if (_bAllowFlying)
	{
		i_math::vector3df pos3D;
		pos3D=LevelUtil_GetWalkableGroundHeight(_level,pos.x,pos.y,TRUE);
		pos3D.setXZ(pos);
		pos3D.y+=ht;

		if (dt>0)
			_vel=(pos3D-_pos3D)/ANIMTICK_TO_SECOND(dt);
		_pos3D=pos3D;
	}

	_pos=pos;
	_ht=ht;
	_face=face;
}



void CSkillGesture_PathS::Update(CUnit3D *unit,float dt)
{
	if (_bFinished)
		return;

	unit->_pos=_pos3D;
	unit->_face=_face;
	unit->_vel=_vel;
}

void CSkillGesture_PathS::Update(CUnit *unit,float dt)
{
	if (_bFinished)
		return;

	unit->_pos=_pos;
	unit->_face=_face;
}



//////////////////////////////////////////////////////////////////////////
//CSkill_General
BIND_SKILLPARAM(Skill_GeneralS,SkillParam_GeneralS);

void Skill_GeneralS::_CalcAnimXfm(i_math::xformf &xfm,AnimTick t)
{
	if (_path)
	{
		Key_pos k;
		Key_f k2;
		if (_path->ksPos3D.CalcKey(t,&k))
		{
			if (KeySet_CalcAngleKey(&_path->ksFace,&k2,t))
			{
				LevelFaceToQuat(k2.v,xfm.rot);
				xfm.pos=k.v*_owner->GetModelScale();
			}
		}
	}
}

BOOL Skill_GeneralS::_WriteSyncData(CBitPacket *bp)
{
	bp->Data_WriteSimpleR(_pos);
	bp->Data_WriteSimple(_ht);
	bp->Data_WriteSimple(_face);
	bp->Data_WriteSimple(_tCasting);

	if (TRUE)
	{
		LevelMoveMethod methodMove=LevelMoveMethod_None;
		CLevelObjMove *move=_owner->GetMove();
		if (move)
			methodMove=move->GetMethod();

		bp->Bits_Write(methodMove,4);
	}

	return TRUE;
}


void Skill_GeneralS::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);
	_SetState(SkillState_Casting);


	BOOL bOk=FALSE;
	if (TRUE)
	{
		SkillParam_GeneralS*param=_rec->GetParam<SkillParam_GeneralS>();
		if (param)
		{
			CUnit *unit=_owner->GetUnit();
			if (unit)
			{
				LevelPathes *pathes=NULL;
				if (TRUE)
				{
					CLevelResources *res=GetLevel()->GetResources();
					if (res)
						pathes=res->FindPathes(param->idPathRes);
				}

				if (pathes)
				{
					if (pathes->def)
					{
						_dur=pathes->def->dur;
						assert(_dur>0);
					}
				}

				if (param->dur>0)
					_dur=param->dur;

				if (pathes)
				{
					_events.Init(&param->entriesEo,&param->entriesOp);
					_events.SetEvents(&pathes->events,_tCasting);
				}

				_path=pathes->def;

				_tXfmAnim=_tCasting;
				_CalcAnimXfm(_xfmAnim,_tCasting);

				//_pos and _ht
				if (TRUE)
				{
					LevelPos3D pos3D=_owner->GetFramePos3D();

					//_pos
					_pos=_owner->GetFramePos3D().getXZ();

					//_ht
					if (TRUE)
					{
						float htAnim=_xfmAnim.pos.y;
						if (htAnim<0.001f)
							_ht=0.0f;//动画里是贴地的,我们强制单位为贴地
						else
						{
							LevelPos3D posGround=LevelUtil_GetWalkableGroundHeight(GetLevel(),pos3D.x,pos3D.z,TRUE);
							_ht=pos3D.y-posGround.y;
							if (_ht<0.0f)
								_ht=0.0f;
						}
					}
				}
				
				//_face
				extern LevelFace LevelUtil_CalcTargetFacing(LevelFace faceInitial,CLevelObj *lo,LevelSkillTarget &target,LevelSkillTargetFacingMode mode,float angleMaxAdjust);
				_face=LevelUtil_CalcTargetFacing(_owner->GetFrameFace(),_owner,_target,param->modeInitialFacing,param->angleMaxInitialFacingAdjust);

				//_facePath
				if (TRUE)
				{
					_facePath=_owner->GetFrameFace();
					switch(param->modePathMatch)
					{
						case SkillParam_General::PathMatchMode_TargetDirection:
						case SkillParam_General::PathMatchMode_TargetPos:
						{
							float faceMatched=_facePath;
							extern LevelFace LevelUtil_CalcTargetFacing(LevelFace faceInitial,CLevelObj *lo,LevelSkillTarget &target,LevelSkillTargetFacingMode mode,float angleMaxAdjust);

							float angleMaxAdjust=180.0f;
							if (param->modePathMatch==SkillParam_General::PathMatchMode_TargetDirection)
								angleMaxAdjust=param->angleMaxInitialPathFacingAdjust;
							if (_target.tp!=LevelSkillTarget::Target_FixPosAndObj)
								_facePath=LevelUtil_CalcTargetFacing(_owner->GetFrameFace(),_owner,_target,LevelSkillTargetFacingMode_FaceTarget,angleMaxAdjust);
							else
								_facePath=LevelUtil_CalcTargetFacing(_owner->GetFrameFace(),_owner,_target,LevelSkillTargetFacingMode_FaceTargetFixedPos,angleMaxAdjust);

							break;
						}
					}
				}

				if (pathes->def)
				{
					BOOL bAllowFlying=FALSE;
					if (_owner->GetType()==LevelObjType_Unit)
					{
						LevelRecordUnit *recUnit=((CLoUnit *)_owner)->GetRec();
						if (recUnit)
							bAllowFlying=recUnit->fly.bEnable;
					}
					_ges=Class_New2(CSkillGesture_PathS);
					_ges->Create(GetLevel(),bAllowFlying);
					_ges->AddRef();

					_ges->ResetLoc(_pos,_ht,_face);

					unit->SetGesture(_ges);
				}

				//_scaleZAxis
				if (TRUE)
				{
					_scaleZAxis=1.0f;
					if (param->modePathMatch==SkillParam_General::PathMatchMode_TargetPos)
					{
						float distToTarget=0.0f;
						if (TRUE)
						{
							LevelPos posTarget;
							if (_target.tp!=LevelSkillTarget::Target_FixPosAndObj)
								LevelUtil_CalcTargetPos(_owner->GetLevel(),_target,posTarget);
							else
								posTarget=_target.Pos();

							distToTarget=_owner->GetFramePos().getDistanceFrom(posTarget);
						}

						float distAnim=0.0f;
						if (TRUE)
						{
							Key_pos kStart,kEnd;
							_path->ksPos3D.CalcKey(_path->ksPos3D.GetStartTick(),&kStart);
							_path->ksPos3D.CalcKey(_path->ksPos3D.GetEndTick(),&kEnd);
							distAnim=kEnd.v.getDistanceXZFrom(kStart.v);
						}

						if (distAnim<=0.001f)
							_scaleZAxis=0.0f;
						else
							_scaleZAxis=distToTarget/distAnim;
					}
				}

				//_scaleFaceCurRotate
				if (TRUE)
				{
					_scaleFaceCurRotate=1.0f;

					if (param->modeFinalFacing!=LevelSkillTargetFacingMode_None)
					{
						extern LevelFace LevelUtil_CalcTargetFacing(LevelFace faceInitial,CLevelObj *lo,LevelSkillTarget &target,LevelSkillTargetFacingMode mode,float angleMaxAdjust);
						float face=LevelUtil_CalcTargetFacing(_owner->GetFrameFace(),_owner,_target,param->modeFinalFacing,180.0f);
						float faceOff=face-_face;
						faceOff=i_math::normalize_radian(faceOff);

						float faceOffPath=0.0f;
						if (TRUE)
						{
							Key_f kStart,kEnd;
							KeySet_CalcAngleKey(&_path->ksFace,&kStart,_path->ksFace.GetStartTick());
							KeySet_CalcAngleKey(&_path->ksFace,&kEnd,_path->ksFace.GetEndTick());

							faceOffPath=i_math::normalize_radian(kEnd.v-kStart.v);
						}

						if (fabsf(faceOffPath)<=0.01f)
							_scaleFaceCurRotate=0.0f;
						else
							_scaleFaceCurRotate=faceOff/faceOffPath;
					}
				}



				//_scaleHt
				if (TRUE)
				{
					float htAnim=_xfmAnim.pos.y;
					if (htAnim<=0.001f)
						_scaleHt=1.0f;
					else
						_scaleHt=_ht/htAnim;
				}

				_methodObstacle=param->methodObstacle;

				bOk=TRUE;
			}
		}
	}

	if (!bOk)
		_SetState(SkillState_Fail);


	_AddSyncDataOp();

	_events.Update(this,_tCasting);

//	_OnUpdate(LEVEL_SKILL_UPDATE_TICK);
}

LevelFace Skill_GeneralS::_AdjustFacing(LevelFace face,LevelSkillTargetFacingMode mode,float speedMaxAdjust,AnimTick dt)
{
	if (mode!=LevelSkillTargetFacingMode_None)
	{
		extern LevelFace LevelUtil_CalcTargetFacing(LevelFace faceInitial,CLevelObj *lo,LevelSkillTarget &target,LevelSkillTargetFacingMode mode,float angleMaxAdjust);
		LevelFace faceNew=LevelUtil_CalcTargetFacing(_owner->GetFrameFace(),_owner,_target,mode,180.0f);

		LevelFace limit=speedMaxAdjust*i_math::GRAD_PI2*ANIMTICK_TO_SECOND(dt);

		i_math::rotate_limited(face,faceNew,limit);
	}
	return face;
}



void Skill_GeneralS::_OnUpdate(AnimTick dt)
{
	CLevel *level=GetLevel();
	if (!level)
		return;
	if (!_owner)
		return;

	if (_state==SkillState_Casting)
	{
		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		SkillParam_GeneralS*param=_rec->GetParam<SkillParam_GeneralS>();

		_events.ClearFrameHistory();

		if (_tCasting>_tXfmAnim)
		{
			i_math::xformf xfmAnimNext;
			
			_CalcAnimXfm(xfmAnimNext,_tCasting);

			//修正_facePath
			_facePath=_AdjustFacing(_facePath,_modePathFacing,param->speedMaxPathFacingAdjust,dt);

			//Make the step
			LevelPos3D posDelta;
			LevelFace faceDelta=0.0f;
			if (TRUE)
			{
				//将xfmNext.pos转换到_xfmAnim的局部空间里,保存为posDelta
				if (TRUE)
				{
					i_math::matrix43f mat;
					_xfmAnim.getMatrix(mat);
					mat.makeInverse();
					mat.transformVect(xfmAnimNext.pos,posDelta);
				}

				if (param->modePathMatch==SkillParam_General::PathMatchMode_TargetPos)
					posDelta.z*=_scaleZAxis;

				//将posDelta转到Path的朝向上
				if (TRUE)
				{
					i_math::xformf xfmPathRot;
					LevelFaceToQuat(_facePath,xfmPathRot.rot);

					i_math::matrix43f matPathRot;
					xfmPathRot.getMatrix(matPathRot);
					matPathRot.transformVect(posDelta,posDelta);
				}

				LevelFace face=LevelFaceFromQuat(xfmAnimNext.rot);
				LevelFace faceLast=LevelFaceFromQuat(_xfmAnim.rot);
				faceDelta=i_math::normalize_radian(face-faceLast)*_scaleFaceCurRotate;
			}

			LevelFace faceNext;
			LevelPos posNext;
			float htNext;

			//Apply the step(face)
			if (TRUE)
			{
				faceNext=_face+faceDelta;
				if (_modeFacing!=LevelSkillTargetFacingMode_None)
					faceNext=_AdjustFacing(_face,_modeFacing,param->speedMaxFacingAdjust,dt);
			}

			//Apply the step(pos)
			if (_methodObstacle!=SkillParam_General::ObstacleMethod_NotCheck)
			{
				i_math::line2df step;
				step.start=_pos;
				step.end=_pos+posDelta.getXZ();
				i_math::vector2df vRay=step.end-step.start;
				float dist=vRay.getLength();
				float distOrg=dist;

				//Static obstacle check
				if (TRUE)
				{
					CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();
					if (unitmgr)
					{
						LevelPos posHit;
						if (unitmgr->StaticRayCast(UnitFindPath_Walkable,step.start,step.end,posHit))
						{
							step.end=posHit;
							vRay=step.end-step.start;
							dist=vRay.getLength();
						}
					}
				}

				//Dyn obstacle check
				if (TRUE)
				{
					CLevelObjMap *mpObj=level->GetObjMap();

					float radiusSrc=_owner->GetRadius_();
					if (dist>=0.001f)
					{
						vRay/=dist;

						LevelDetectTargetFlag flagDetect=(LevelDetectTargetFlag)(LevelDetectTarget_Enemy|LevelDetectTarget_Unit|LevelDetectTarget_Player|LevelDetectTarget_Ground);
						LevelObjRequire require=LevelObjRequire_Attackable;
						LevelUtilDetectParam paramDetect;
						paramDetect.flags=&flagDetect;
						paramDetect.nFlags=1;
						paramDetect.requires=&require;
						paramDetect.nRequires=1;
						paramDetect.loSrc=_owner;
						paramDetect.pos=step.getMiddle();
						paramDetect.rangeMin=0.0f;
						paramDetect.bTouching=TRUE;
						paramDetect.rangeMax=((float)step.getLength())/2.0f+radiusSrc;

						DWORD c;
						extern CLevelObj **LevelUtil_Detect(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,DWORD &c);
						CLevelObj **buf=LevelUtil_Detect(paramDetect,NULL,c);

						extern float intersectSphereBySphere(i_math::vector2df& vSrc,float rSrc,i_math::vector2df& vRay, i_math::vector2df& vTarget, float rTarget);


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
							float distHit=intersectSphereBySphere(step.start,radiusSrc,vRay,posTarget,radiusTarget);
							if ((distHit>=0.0f)&&(distHit<dist))
							{
								if (_methodObstacle==SkillParam_General::ObstacleMethod_StopAtStaticObstacleAndBumpEnemyObstacle)
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
										dirKB.y=-vRay.x;
										dirKB.x=vRay.y;
									}


									BuffArg_PB arg;
									arg.posTarget=loTarget->GetFramePos()+dirKB*distKB;
									arg.face=atan2f(-dirKB.y,-dirKB.x);

									LevelOpLink link;
									link.id=GetLevel()->GenOpLinkID();
									link.t=level->GetT_();

									level->GetDecider()->MakeBuff(LevelOSB(this),loTarget,level->GetRecords()->GetGlobal()->idDefBuff_PB,0,&arg,link);
								}
								if (_methodObstacle==SkillParam_General::ObstacleMethod_StopAtStaticObstacleOrEnemyObstacle)
								{
									step.end=step.start+vRay*distHit;
									dist=distHit;
// 									GetLevel()->GetDbgDraw().DrawCircle(step.start,radiusSrc,RGB(0,255,255),2.0f);
// 									GetLevel()->GetDbgDraw().DrawCircle(posTarget,radiusTarget,RGB(255,0,255),2.0f);
// 									GetLevel()->GetDbgDraw().DrawCircle(step.end,0.03f,RGB(255,0,0),2.0f);
								}
							}
						}
					}
				}
				if (dist<distOrg)
				{
					float dist3D=posDelta.getLength();
					if (dist3D>0.001f)
						dist3D*=dist/distOrg;
					posDelta.setLength(dist3D);
				}
			}

			posNext=_pos+posDelta.getXZ();
			htNext=_ht+posDelta.y*_scaleHt;

			if (_ges)
				_ges->UpdateLoc(posNext,htNext,faceNext,dt);

			_pos=posNext;
			_face=faceNext;
			_ht=htNext;
			_AddSyncDataOp();
			GetLevel()->AddAffect(_owner);

			_xfmAnim=xfmAnimNext;
			_tXfmAnim=_tCasting;
		}

		_events.Update(this,_tCasting);

		if (_tCasting>=_dur)
			_Finish();
	}
}

void Skill_GeneralS::_OnBreak()
{
	_Finish();
}


void Skill_GeneralS::_Finish()
{
	if (_ges)
		_ges->Stop();
	SAFE_RELEASE(_ges);

	if (_owner)
	{
		LevelAttr_WeaksMod *mod=_owner->GetAttr_WeaksMod();
		if (mod)
			mod->ClearOverride(this,_owner->GetT());
	}

	_events.Clear();

	_SetState(SkillState_Finished);
}

AnimTick Skill_GeneralS::GetCastingEventTime(StringID nmEvent)
{
	return _events.GetCastingEventTime(nmEvent);
}

void Skill_GeneralS::OnOp(GeneralSkillOpEntry &entryOp)
{
	switch(entryOp.op)
	{
		case GeneralSkillOpEntry::Op_SetFacingModeToNone:
		{
			_modeFacing=LevelSkillTargetFacingMode_None;
			break;
		}
		case GeneralSkillOpEntry::Op_SetFacingModeToFaceTarget:
		{
			_modeFacing=LevelSkillTargetFacingMode_FaceTarget;
			break;
		}
		case GeneralSkillOpEntry::Op_SetFacingModeToFaceTargetFixedPos:
		{
			_modeFacing=LevelSkillTargetFacingMode_FaceTargetFixedPos;
			break;
		}
		case GeneralSkillOpEntry::Op_SetPathFacingModeToNone:
		{
			_modePathFacing=LevelSkillTargetFacingMode_None;
			break;
		}
		case GeneralSkillOpEntry::Op_SetPathFacingModeToFaceTarget:
		{
			_modePathFacing=LevelSkillTargetFacingMode_FaceTarget;
			break;
		}
		case GeneralSkillOpEntry::Op_AllowCancel:
		{
			_bAllowCancel=TRUE;
			break;
		}
		case GeneralSkillOpEntry::Op_OverrideWeaks:
		{
			if (_owner)
			{
				LevelAttr_WeaksMod *mod=_owner->GetAttr_WeaksMod();
				if (mod)
				{
					LevelWeaksPack wkpk;
					entryOp.weaks.ToWeakPack(wkpk);
					mod->SetOverride(wkpk,this,_owner->GetT(),TRUE);
				}
			}
			break;
		}
		case GeneralSkillOpEntry::Op_CleanOverrideWeaks:
		{
			if (_owner)
			{
				LevelAttr_WeaksMod *mod=_owner->GetAttr_WeaksMod();
				if (mod)
					mod->ClearOverride(this,_owner->GetT());
			}
			break;
		}
		case GeneralSkillOpEntry::Op_TakeOff:
		{
			CLevelObjMove *move=_owner->GetMove();
			if (move)
			{
				if (move->GetMethod()!=LevelMoveMethod_Flying)
				{
					LevelPos3D pos=_owner->GetFramePos3D();
					LevelFace face=_owner->GetFrameFace();
					move->SwitchFlying(pos,face,LevelTeleportID_Invalid);
				}
			}
			break;
		}
		case GeneralSkillOpEntry::Op_Landing:
		{
			CLevelObjMove *move=_owner->GetMove();
			if (move)
			{
				if (move->GetMethod()!=LevelMoveMethod_Ground)
				{
					LevelPos3D pos=_owner->GetFramePos3D();
					LevelFace face=_owner->GetFrameFace();
					move->SwitchGround(pos.getXZ(),face,LevelTeleportID_Invalid);
				}
			}
			_scaleHt=1.0f;
			break;
		}

	}
}

void Skill_GeneralS::Cancel()
{
// 	if (!_bAllowCancel)
// 		return;
	_Finish();
}

BOOL Skill_GeneralS::CheckEventWindow(StringID nmOpen,StringID nmClose)
{
	if (_state==SkillState_Casting)
		return _events.CheckWindow(nmOpen,nmClose,_tCasting);
	return FALSE;
}
