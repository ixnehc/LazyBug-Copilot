/********************************************************************
	created:	2016/09/02 
	author:		cxi
	
	purpose:	通用的Skill
*********************************************************************/
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "Skill_GeneralS.h" 
#include "Skill_GeneralAdvS.h"

#include "LevelRecordSkill.h" 
#include "LevelRecordUnit.h"

#include "LoUnit.h"
#include "Level.h" 
#include "LevelBuff.h"

#include "LoEffectObj.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "LevelBehavior.h"

#include "LevelDecider.h"
#include "LevelRecords.h"
#include "LevelRecordGlobal.h"

#include "LevelResources.h"

#include "LevelObjMove.h"

#include "Buff_PB.h"

#include "Random/Random.h"



//////////////////////////////////////////////////////////////////////////
//CSkill_General
BIND_SKILLPARAM(Skill_GeneralAdvS,SkillParam_GeneralAdvS);

void Skill_GeneralAdvS::_CalcAnimXfm(i_math::xformf &xfm,AnimTick t)
{
	xfm=i_math::xformf();
	if (_stage.path)
	{
		AnimTick tStage=ANIMTICK_SAFE_MINUS(t,_stage.tStart);
		Key_pos k;
		Key_f k2;
		if (_stage.path->ksPos3D.CalcKey(tStage,&k))
		{
			if (KeySet_CalcAngleKey(&_stage.path->ksFace,&k2,tStage))
			{
				LevelFaceToQuat(k2.v,xfm.rot);
				xfm.pos=k.v*_owner->GetModelScale();
			}
		}
	}
}

void Skill_GeneralAdvS::GetCastingPos(LevelPos &pos)
{
	if (_ges)
		pos=_ges->_pos;
	else
		pos=_owner->GetFramePos();
}

void Skill_GeneralAdvS::GetCastingPos3D(LevelPos3D &pos3D)
{
	if (_ges)
	{
		if (_ges->_bAllowFlying)
			pos3D=_ges->_pos3D;
		else
		{
			pos3D=LevelUtil_GetWalkableGroundHeight(_owner->GetLevel(),_ges->_pos.x,_ges->_pos.y,TRUE);
			pos3D.setXZ(_ges->_pos);
			pos3D.y+=_ges->_ht;
		}
	}
	else
		pos3D=_owner->GetFramePos3D();
}


LevelFace Skill_GeneralAdvS::GetCastingFace()
{
	if (_ges)
		return _ges->_face;
	return _owner->GetFrameFace();
}



BOOL Skill_GeneralAdvS::_WriteSyncData(CBitPacket *bp)
{
	bp->Data_WriteSimpleR(_pos);
	bp->Data_WriteSimple(_ht);
	bp->Data_WriteSimple(_face);
	bp->Data_WriteSimple(_tCasting);
	bp->Bits_Write(_stage.idxParam,4);
	bp->Bits_Write(_stage.ver%16,4);

	if (TRUE)
	{
		LevelMoveMethod methodMove=LevelMoveMethod_None;
		CLevelObjMove *move=_owner->GetMove();
		if (move)
			methodMove=move->GetMethod();

		bp->Bits_Write(methodMove,4);
	}

	bp->Bit_Write(_bFinishing);

	return TRUE;
}

void Skill_GeneralAdvS::_OnStart()
{
	_AddStartOp();
	_OnStart_Internal();

}

void Skill_GeneralAdvS::_OnStart(LevelOpLink &link)
{
	_AddStartOp(link);
	_OnStart_Internal();
}


void Skill_GeneralAdvS::_OnStart_Internal()
{
	GetLevel()->AddAffect(_owner);
	_SetState(SkillState_Casting);

	BOOL bOk=FALSE;
	if (TRUE)
	{
		SkillParam_GeneralAdvS*param=_rec->GetParam<SkillParam_GeneralAdvS>();
		if (param)
		{
			_methodObstacle=param->methodObstacle;

			_events.Init(&param->entriesEo,NULL);

			//_pos and _ht
			if (TRUE)
			{
				LevelPos3D pos3D=_owner->GetFramePos3D();
				//_pos
				_pos=pos3D.getXZ();

				//_ht
				LevelPos3D posGround=LevelUtil_GetWalkableGroundHeight(GetLevel(),pos3D.x,pos3D.z,TRUE);
				_ht=pos3D.y-posGround.y;
				if (_ht<0.0f)
					_ht=0.0f;
			}

			//_face and _faceTargetAligner
			_face=_faceTargetAligner=_owner->GetFrameFace();

			if (!_TriggerBhvSwitcher())
			{
				if (param->stages.size()>0)
					_DoSwitchStage(0);
			}
			
			if (TRUE)
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

				CUnit *unit=_owner->GetUnit();
				CUnit3D *unit3D=_owner->GetUnit3D();
				if (unit)
					unit->SetGesture(_ges);
				else
				{
					if (unit3D)
						unit3D->SetGesture(_ges);
				}
			}

			bOk=TRUE;
		}
	}

	if (!bOk)
		_SetState(SkillState_Fail);

	_AddSyncDataOp();

	_UpdateEvents();

//	_OnUpdate(LEVEL_SKILL_UPDATE_TICK);
}

LevelFace Skill_GeneralAdvS::_AdjustFacing(LevelFace face,LevelSkillTargetFacingMode mode,float speedMaxAdjust,AnimTick dt,LevelFaceYaw yaw)
{
	if (mode!=LevelSkillTargetFacingMode_None)
	{
		extern LevelFace LevelUtil_CalcTargetFacing(LevelFace faceInitial,CLevelObj *lo,LevelSkillTarget &target,LevelSkillTargetFacingMode mode,float angleMaxAdjust);
		LevelFace faceNew=LevelUtil_CalcTargetFacing(_owner->GetFrameFace(),_owner,_target,mode,180.0f);

		LevelFaceApplyYaw(faceNew,yaw);

		LevelFace limit=speedMaxAdjust*i_math::GRAD_PI2*ANIMTICK_TO_SECOND(dt);

		i_math::rotate_limited(face,faceNew,limit);
	}
	return face;
}

void Skill_GeneralAdvS::_UpdateEvents()
{
	_events.Update(this,_tCasting);
}


void Skill_GeneralAdvS::_OnUpdate(AnimTick dt)
{
	CLevel *level=GetLevel();
	if (!level)
		return;
	if (!_owner)
		return;

	if (_state==SkillState_Casting)
	{
		_events.ClearFrameHistory();
		if (TRUE)
		{
			AnimTick tCastingOld=_tCasting;
			LevelUtil_AccumCastingTime(_owner,dt,_tCasting);
			dt=ANIMTICK_SAFE_MINUS(_tCasting,tCastingOld);
		}

		_idRecentDynObstacle=LevelObjID_Invalid;

		SkillParam_GeneralAdvS*param=_rec->GetParam<SkillParam_GeneralAdvS>();

		DWORD verOld=_stage.ver;

		_UpdateEvents();

		_UpdateStage(_tCasting,dt);

		if ((_stage.dur!=ANIMTICK_INFINITE)&&(_tCasting>=_stage.tStart+_stage.dur))
		{
			if (!_TriggerBhvSwitcher())
				_bFinishing=TRUE;
		}
		else
		{
			if (_stage.param)
			{
				if (_stage.param->nmUpdater!=StringID_Invalid)
					_CallBehaviorRelay(_stage.param->nmUpdater,NULL);
			}
		}
		if (_stage.ver!=verOld)
			_UpdateEvents();

		_AddSyncDataOp();
		level->AddAffect(_owner);

		if (_bFinishing)
		{
			_CloseAllWindows();
			_Finish();
		}
	}
}

void Skill_GeneralAdvS::_DoSwitchStage(int iStage)
{
	SkillParam_GeneralAdvS*param=_rec->GetParam<SkillParam_GeneralAdvS>();

	BOOL bFirstStage=_stage.IsEmpty();

	_EndStage();


	_StartStage(&param->stages[iStage],iStage,_tCasting);

	_tXfmAnim=_tCasting;
	_CalcAnimXfm(_xfmAnim,_tCasting);

	//_ht
	if (bFirstStage)
	{
		float htAnim=_xfmAnim.pos.y;
		if (htAnim<0.001f)
			_ht=0.0f;//动画里是贴地的,我们强制单位为贴地
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
}


void Skill_GeneralAdvS::DoSwitchStage(StringID nmStage)
{
	if (nmStage==StringID_Invalid)
		_bFinishing=TRUE;
	else
	{
		SkillParam_GeneralAdvS*param=_rec->GetParam<SkillParam_GeneralAdvS>();
		for (int i=0;i<param->stages.size();i++)
		{
			if (param->stages[i].nm==nmStage)
			{
				_DoSwitchStage(i);
				break;
			}
		}
	}

}


BOOL Skill_GeneralAdvS::_TriggerBhvSwitcher()
{
	SkillParam_GeneralAdvS::Stage *paramOld=_stage.param;
	AnimTick tStartOld=_stage.tStart;

	SkillParam_GeneralAdvS*param=_rec->GetParam<SkillParam_GeneralAdvS>();
	if (param)
	{
		if (param->nmStageSwitcher!=StringID_Invalid)
		{
			_CallBehaviorRelay(param->nmStageSwitcher,NULL);
			if ((paramOld!=_stage.param)||(tStartOld!=_stage.tStart))
				return TRUE;
		}
	}

	return FALSE;
}


void Skill_GeneralAdvS::_OnBreak()
{
	_CloseAllWindows();

	_Finish();
}

void Skill_GeneralAdvS::StopCast(AnimTick tStop)
{
	_CloseAllWindows();
	_Finish();
}


void Skill_GeneralAdvS::_Finish()
{
	_EndStage();

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
	_stage.Zero();

	_SetState(SkillState_Finished);
}

AnimTick Skill_GeneralAdvS::GetCastingEventTime(StringID nmEvent)
{
	return _events.GetCastingEventTime(nmEvent);
}

void Skill_GeneralAdvS::_CleanOverrideWeaks()
{
	if (_owner)
	{
		LevelAttr_WeaksMod *mod=_owner->GetAttr_WeaksMod();
		if (mod)
			mod->ClearOverride(this,_owner->GetT());
	}
}


void Skill_GeneralAdvS::_OnOp(SkillParam_GeneralAdvS::OpEntryBase &entryOp,AnimEventZone *ezone,LevelOpLink *link)
{
	switch(entryOp.op)
	{
		case SkillParam_GeneralAdvS::OpEntry::Op_BgHandler:
		{
			if (entryOp.nmBgHandler!=StringID_Invalid)
			{
				_CallBehaviorRelay(entryOp.nmBgHandler,link);
			}
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_AllowCancel:
		{
			_bAllowCancel=TRUE;
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_OverrideWeaks:
		case SkillParam_GeneralAdvS::OpEntry::Op_AddWeaks:
		case SkillParam_GeneralAdvS::OpEntry::Op_RemoveWeaks:
		{
			if (_owner)
			{
				LevelAttr_WeaksMod *mod=_owner->GetAttr_WeaksMod();
				if (mod)
				{
					LevelWeaksPack wkpk;
					LevelWeaksPack wkpkMod;
					entryOp.weaks.ToWeakPack(wkpkMod);
					wkpk=wkpkMod;

					if (entryOp.op==SkillParam_GeneralAdvS::OpEntry::Op_AddWeaks)
					{
						LevelAttr_Weaks *attrWeaks=_owner->GetAttr_Weaks();
						if (attrWeaks)
						{
							wkpk=attrWeaks->Cur();
							wkpk.MergeFrom(wkpkMod);
						}
					}
					if (entryOp.op==SkillParam_GeneralAdvS::OpEntry::Op_RemoveWeaks)
					{
						LevelAttr_Weaks *attrWeaks=_owner->GetAttr_Weaks();
						if (attrWeaks)
						{
							wkpk=attrWeaks->Cur();
							wkpk.Exclude(wkpkMod);
						}
					}

					if (ezone)
					{
						if (ezone->GetDur()>0)
						{
							if (!_stage.IsEmpty())
								_stage.tAutoCleanOverrideWeaks=_tCasting+ezone->GetDur();
						}
					}

					mod->SetOverride(wkpk,this,_owner->GetT(),entryOp.bWeaksCanTakeOver);
				}
			}
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_CleanOverrideWeaks:
		{
			_CleanOverrideWeaks();
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_TakeOff:
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
		case SkillParam_GeneralAdvS::OpEntry::Op_Landing:
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
		case SkillParam_GeneralAdvS::OpEntry::Op_SetFacingMode_None:
		{
			_modeFacing=LevelSkillTargetFacingMode_None;
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_SetPathFacingMode_None:
		{
			_modePathFacing=LevelSkillTargetFacingMode_None;
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_SetFacingMode_Target:
		{
			_modeFacing=LevelSkillTargetFacingMode_FaceTarget;
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_SetPathFacingMode_Target:
		{
			_modePathFacing=LevelSkillTargetFacingMode_FaceTarget;
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_SetPathFacingMode_TargetFixedPos:
		{
			_modePathFacing=LevelSkillTargetFacingMode_FaceTargetFixedPos;
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_OpenWindow:
		{
			_OpenWindow(entryOp.nmWindow);
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_CloseWindow:
		{
			_CloseWindow(entryOp.nmWindow);
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_DisableTargetMatching:
		{
			_bTargetMatching=FALSE;
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_SetObstacleMethod:
		{
			_methodObstacle=entryOp.methodObstacle;
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_AddBuff:
		{
			if (_owner)
			{
				LevelOpLink link;
				link.id=_owner->GetLevel()->GenOpLinkID();
				link.t=_tCasting;
				CLevelDecider *decider=_owner->GetLevel()->GetDecider();
				for (int i=0;i<entryOp.buffs.size();i++)
				{
					LevelBuffID idBuff=decider->MakeBuff(LevelOSB(this),_owner,entryOp.buffs[i],ANIMTICK_INFINITE,NULL,link);
					if (idBuff!=LevelBuffID_Invalid)
					{
						if (entryOp.bManageBuffDur)
						{
							if (ezone)
							{
								if (ezone->GetDur()>0)
								{
									StageState::EventBuff entry;
									entry.idBuff=idBuff;
									entry.tAutoClean=_tCasting+ezone->GetDur();
									_stage.buffsEvent.push_back(entry);
								}
							}
						}
					}
				}
			}
			break;
		}
		case SkillParam_GeneralAdvS::OpEntry::Op_RemoveBuff:
		{
			if (_owner)
			{
				LevelOpLink link;
				link.id=_owner->GetLevel()->GenOpLinkID();
				link.t=_tCasting;
				CLevelDecider *decider=_owner->GetLevel()->GetDecider();
				for (int i=0;i<entryOp.buffs.size();i++)
				{
					CLevelBuff *buff=LevelUtil_FindBuffByRecordID(_owner,entryOp.buffs[i]);
					if (buff)
						decider->RemoveBuff(LevelOSB(this),_owner,buff,link);
				}
			}
			break;
		}

	}
}

void Skill_GeneralAdvS::Cancel()
{
// 	if (!_bAllowCancel)
// 		return;
	_CloseAllWindows();
	_Finish();
}

BOOL Skill_GeneralAdvS::CheckEventWindow(StringID nmOpen,StringID nmClose)
{
	if (_state==SkillState_Casting)
		return _events.CheckWindow(nmOpen,nmClose,_tCasting);
	return FALSE;
}

void Skill_GeneralAdvS::OnEvent(StringID nmEvent,AnimEventZone *ezone,AnimTick t)
{
	SkillParam_GeneralAdvS*param=_rec->GetParam<SkillParam_GeneralAdvS>();
	if (param)
	{
		for (int i=0;i<param->entriesOp.size();i++)
		{
			if (!param->entriesOp[i].bEnable)
				continue;
			if (param->entriesOp[i].nm!=nmEvent)
				continue;

			_OnOp(param->entriesOp[i],ezone,NULL);
		}
	}

}

BOOL Skill_GeneralAdvS::_CalcTargetAlignerFace(AnimTick t,LevelFace faceBase,LevelFace &faceTargetAligner)
{
	if (_stage.ezoneTargetAligner)
	{
		AnimEventZone::KeyFan kFan;
		if(_stage.ezoneTargetAligner->IsIn(t))
		{
			if (_stage.ezoneTargetAligner->CalcKeyFan(t,kFan))
			{
				i_math::vector3df pos;
				i_math::vector3df dir;
				float fov;
				if (!kFan.CalcInfo(pos,dir,fov))
					return FALSE;

				faceTargetAligner=LevelFaceFromDir(dir.getXZ());

				LevelFaceYaw yaw=LevelFaceCalcYaw(LevelFaceFromDir(i_math::vector2df(0.0f,1.0f)),faceTargetAligner);
				faceTargetAligner=faceBase;
				LevelFaceApplyYaw(faceTargetAligner,yaw);
				return TRUE;
			}
		}
	}
	return FALSE;
}



void Skill_GeneralAdvS::_StartStage(SkillParam_GeneralAdvS::Stage *paramStage,int idxParam,AnimTick tStart0)
{  
	_stage.ver++;  

	_stage.param=paramStage;
	_stage.idxParam=idxParam;

	_stage.tStart=tStart0;

	if (paramStage->idBuff==RecordID_Invalid)
	{
		LevelPathes *pathes=NULL;
		if (TRUE)
		{
			CLevelResources *res=GetLevel()->GetResources();
			if (res)
				pathes=res->FindPathes(paramStage->idPathRes);
		}

		if (pathes)
		{
			if (pathes->def)
			{
				_stage.path=pathes->def;
				_stage.dur=pathes->def->dur;
				assert(_stage.dur>0);
			}
		}

		if (pathes)
			_events.SetEvents(&pathes->events,tStart0);
	}
	else
	{
		CLevelBuff *buff=LevelUtil_FindBuffByRecordID(_owner,paramStage->idBuff);
		if (buff)
		{
			_stage.idBuff=buff->GetID();
			_stage.dur=ANIMTICK_INFINITE;
			buff->StartSkillPath(_pos,_ht);
		}
	}

	if (paramStage->dur>0)
		_stage.dur=CSysRandom::RandVaryUInt(paramStage->dur,paramStage->durVar);

	AnimTick tStart=0;
	AnimTick tEnd=_stage.dur;

	_stage.htStart=_ht;

	_stage.scaleFaceRotate=1.0f;

	if (paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_AnimDriven_Default)
	{
		SkillParam_GeneralAdvS*param=_rec->GetParam<SkillParam_GeneralAdvS>();
		extern AnimEventZone *LevelUtil_FindEZone(CLevel *level,SkillParam_GeneralAdvS *paramSkill,SkillParam_GeneralAdvS::Stage* paramSkillStage,StringID nmEvent);
		_stage.ezoneTargetAligner=LevelUtil_FindEZone(GetLevel(),param,paramStage,paramStage->nmTargetAlignerEvent);
		if (paramStage->targetmatch.bEnable)
			_stage.ezoneTargetMatcher=LevelUtil_FindEZone(GetLevel(),param,paramStage,paramStage->targetmatch.nmEvent);

	}

	if(paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_AnimDriven_Default)
	{
		_face=_owner->GetFrameFace();

		//adjust on _face
		if (TRUE)
		{
			LevelFace faceTargetAligner;
			if(_CalcTargetAlignerFace(0,_face,faceTargetAligner))
			{
				if (paramStage->modeInitialPathFacing!=LevelSkillTargetFacingMode_None)
				{
					LevelFace faceTargetAlignerAdjusted=LevelUtil_CalcTargetFacing(faceTargetAligner,_owner,_target,paramStage->modeInitialPathFacing,paramStage->angleMaxInitialPathFacingAdjust);
					LevelFaceYaw yaw=LevelFaceCalcYaw(faceTargetAligner,faceTargetAlignerAdjusted);
					LevelFaceApplyYaw(_face,yaw);
				}
			}
		}

		_modeFacing=paramStage->modeFacing;
		_modePathFacing=paramStage->modePathFacing;
	}


	if ((paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_AnimDriven_Default_Old)||
		(paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_CodeDriven_MoveAlong))
	{
		if (paramStage->modeInitialFacing!=LevelSkillTargetFacingMode_None)
			_face=LevelUtil_CalcTargetFacing(_owner->GetFrameFace(),_owner,_target,paramStage->modeInitialFacing,paramStage->angleMaxInitialFacingAdjust);

		//计算_faceTargetAligner的原始值
		if (TRUE)
		{
			LevelFace faceWorld=_owner->GetFrameFace();//当前单位的世界空间里的朝向
			i_math::xformf xfm;
			_CalcAnimXfm(xfm,_tCasting);
			LevelFace faceLocal=LevelFaceFromQuat(xfm.rot);//动画里的朝向
			LevelFace faceFrontLocal=LevelFaceFromDir(i_math::vector2df(0.0f,1.0f));//动画里正前方的朝向
			LevelFace faceFrontWorld=faceWorld+(faceFrontLocal-faceLocal);//世界空间里的正前方的朝向
			_faceTargetAligner=faceFrontWorld;
			if (_stage.path)
				LevelFaceApplyYaw(_faceTargetAligner,_stage.path->facePath*i_math::GRAD_PI2);
		}

		//Adjust _faceTargetAligner
		if (paramStage->modeInitialPathFacing!=LevelSkillTargetFacingMode_None)
			_faceTargetAligner=LevelUtil_CalcTargetFacing(_faceTargetAligner,_owner,_target,paramStage->modeInitialPathFacing,paramStage->angleMaxInitialPathFacingAdjust);

		_modeFacing=paramStage->modeFacing;
		_modePathFacing=paramStage->modePathFacing;
	}

	if (paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_AnimDriven_RotateOnSpot)
	{
		extern LevelFace LevelUtil_CalcTargetFacing(LevelFace faceInitial,CLevelObj *lo,LevelSkillTarget &target,LevelSkillTargetFacingMode mode,float angleMaxAdjust);
		float face=LevelUtil_CalcTargetFacing(_owner->GetFrameFace(),_owner,_target,paramStage->modeRotateOnSpotFacing,180.0f);
		float faceOff=face-_face;
		faceOff=i_math::normalize_radian(faceOff);

		float faceOffPath=0.0f;
		if (_stage.path)
		{
			Key_f kStart,kEnd;
			KeySet_CalcAngleKey(&_stage.path->ksFace,&kStart,tStart);
			KeySet_CalcAngleKey(&_stage.path->ksFace,&kEnd,tEnd);

			faceOffPath=i_math::normalize_radian(kEnd.v-kStart.v);
			if ((faceOffPath>=i_math::Pi-0.001f)||(faceOffPath<=-i_math::Pi+0.001f))
			{
				//几乎180度,要判断一下方向
				DWORD nKeys=_stage.path->ksFace.GetKeyCount();
				for (int i=1;i<nKeys;i++)
				{
					Key_f *k=(Key_f *)_stage.path->ksFace.GetKey(i);
					float faceOff=i_math::normalize_radian(k->v-kStart.v);
					if (faceOff>0.01f)
					{
						faceOffPath=i_math::Pi;
						break;
					}
					if (faceOff<0.01f)
					{
						faceOffPath=-i_math::Pi;
						break;
					}
				}
			}
		}

		if (fabsf(faceOffPath)<=0.01f)
			_stage.scaleFaceRotate=0.0f;
		else
			_stage.scaleFaceRotate=faceOff/faceOffPath;
	}

	if (paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_BuffDriven_WS)
	{

	}

	_stage.posStageStart=_pos;
	if (_stage.path)
	{
		Key_pos kStart,kEnd;
		_stage.path->ksPos3D.CalcKey(tStart,&kStart);
		_stage.path->ksPos3D.CalcKey(tEnd,&kEnd);
	}

	_bTargetMatching=paramStage->targetmatchOld.bEnable;

}

void Skill_GeneralAdvS::_ClearEventBuff(AnimTick t,StageState::EventBuff &entry)
{
	CLevelDecider *decider=_owner->GetLevel()->GetDecider();
	LevelOpLink link;
	link.id=_owner->GetLevel()->GenOpLinkID();
	link.t=t;

	CLevelBuff *buff=LevelUtil_FindBuffByID(_owner,entry.idBuff);
	if (buff)
		decider->RemoveBuff(LevelOSB(this),_owner,buff,link);
	entry.Zero();
}


void Skill_GeneralAdvS::_EndStage()
{
	if (_stage.idBuff!=LevelBuffID_Invalid)
	{
		if (CheckOwnerAlive())
		{
			extern CLevelBuff *LevelUtil_FindBuffByID(CLevelObj *lo,LevelBuffID idBuff);
			CLevelBuff *buff=LevelUtil_FindBuffByID(_owner,_stage.idBuff);
			if (buff)
				buff->StopSkillPath();
		}
	}

	if (_stage.tAutoCleanOverrideWeaks>0)
	{
		_CleanOverrideWeaks();
		_stage.tAutoCleanOverrideWeaks=0;
	}

	if (TRUE)
	{
		CLevelDecider *decider=_owner->GetLevel()->GetDecider();
		for (int i=0;i<_stage.buffsEvent.size();i++)
			_ClearEventBuff(_tCasting,_stage.buffsEvent[i]); 
		_stage.buffsEvent.clear();
	}

	_stage.Zero();

	_events.SetEvents(NULL,0);
}

AnimTick Skill_GeneralAdvS::GetStageAge()
{
	if (!ExistStage())
		return 0;
	AnimTick tCur=_owner->GetT();
	return ANIMTICK_SAFE_MINUS(tCur,_stage.tStart);
}



void Skill_GeneralAdvS::_UpdateStage(AnimTick t,AnimTick dt)
{
	CLevel *level=GetLevel();
	if (!_stage.param)
		return;
	SkillParam_GeneralAdvS::Stage *paramStage=_stage.param;
	SkillParam_GeneralAdvS*param=_rec->GetParam<SkillParam_GeneralAdvS>();

	if (_stage.tAutoCleanOverrideWeaks>0)
	{
		if (t>=_stage.tAutoCleanOverrideWeaks)
		{
			_CleanOverrideWeaks();
			_stage.tAutoCleanOverrideWeaks=0;
		}
	}

	if (TRUE)
	{
		int c=0;
		for (int i=0;i<_stage.buffsEvent.size();i++)
		{
			if (t>=_stage.buffsEvent[i].tAutoClean)
			{
				_ClearEventBuff(t,_stage.buffsEvent[i]);
				continue;
			}
			_stage.buffsEvent[c]=_stage.buffsEvent[i];
			c++;
		}
		_stage.buffsEvent.resize(c);
	}


	if (t>_tXfmAnim)
	{
		i_math::xformf xfmAnimNext;

		_CalcAnimXfm(xfmAnimNext,t);

		LevelFace faceNext=_face;
		LevelFace faceTargetAlignerNext=_faceTargetAligner;
		LevelPos posNext;
		float htNext;

		if (TRUE)
		{
			//Make the step
			LevelPos3D posDelta;

			//计算posDelta,faceNext,faceTargetAlignerNext
			if (paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_BuffDriven_WS)
			{
				posDelta.setZero();

				extern CLevelBuff *LevelUtil_FindBuffByID(CLevelObj *lo,LevelBuffID idBuff);
				CLevelBuff *buff=LevelUtil_FindBuffByID(_owner,_stage.idBuff);
				if (buff)
				{
					LevelPos pos;
					LevelFace face;
					float ht;
					if (buff->CalcSkillPathXfm(level->GetT_(),pos,ht,face))
					{
						faceNext=faceTargetAlignerNext=face;
						posDelta.x=pos.x-_pos.x;
						posDelta.z=pos.y-_pos.y;
						posDelta.y=ht-_ht;
					}
				}
			}

			//计算posDelta,faceNext
			if (paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_AnimDriven_Default)
			{
				i_math::xformf xfmDelta=xfmAnimNext;
				xfmDelta.makeLocal(_xfmAnim);

				BOOL bValidTargetAligner=FALSE;
				LevelFace faceTargetAligner;
				LevelFace faceTargetAlignerAdjusted;

				if (paramStage->modePathFacing!=LevelSkillTargetFacingMode_None)
				{
					LevelFace faceDelta=LevelFaceFromQuat(xfmDelta.rot);
					LevelFaceYaw yaw=LevelFaceCalcYaw(LevelFaceFromDir(i_math::vector2df(0.0f,1.0f)),faceDelta);
					LevelFace faceNext=_face;
					LevelFaceApplyYaw(faceNext,yaw);
					if (_CalcTargetAlignerFace(ANIMTICK_SAFE_MINUS(t,_stage.tStart),faceNext,faceTargetAligner))
					{
						faceTargetAlignerAdjusted=_AdjustFacing(faceTargetAligner,_modePathFacing,paramStage->speedMaxPathFacingAdjust,dt,0.0f);
						LevelFaceYaw yaw=LevelFaceCalcYaw(faceTargetAligner,faceTargetAlignerAdjusted);
						LevelFaceApplyYaw(faceDelta,yaw);
						LevelFaceToQuat(faceDelta,xfmDelta.rot);
						bValidTargetAligner=TRUE;
					}
				}

				i_math::xformf xfm;//当前单位的世界空间xfm
				xfm.pos.setXZ(_pos);
				LevelFaceToQuat(_face,xfm.rot);

				i_math::xformf xfmNext=xfmDelta;//下一帧单位的世界空间xfm
				xfmNext.applyBase(xfm);

				faceNext=LevelFaceFromQuat(xfmNext.rot);
				posDelta.setXZ(xfmNext.pos.getXZ()-_pos);
				posDelta.y=xfmDelta.pos.y;

				if (_stage.ezoneTargetMatcher)
				{
					AnimTick tCur=ANIMTICK_SAFE_MINUS(_tXfmAnim,_stage.tStart);
					AnimTick tNext=ANIMTICK_SAFE_MINUS(t,_stage.tStart);
					AnimTick tEnd=_stage.ezoneTargetMatcher->GetEnd();
					if ((tCur>=_stage.ezoneTargetMatcher->GetStart())&&(tCur<tEnd))
					{
						i_math::xformf xfmEnd;//tEnd时单位的世界空间Xfm
						if (tEnd<=tNext)
							xfmEnd=xfmNext;
						else
						{
							_CalcAnimXfm(xfmEnd,_stage.tStart+tEnd);
							xfmEnd.makeLocal(_xfmAnim);
							xfmEnd.applyBase(xfm);
						}

						i_math::xformf xfmEventCur,xfmEventNext,xfmEventEnd;
						if (TRUE)
						{
							_stage.ezoneTargetMatcher->CalcXform(tCur,xfmEventCur);
							xfmEventCur.applyBase(xfm);
							_stage.ezoneTargetMatcher->CalcXform(tNext,xfmEventNext);
							xfmEventNext.applyBase(xfmNext);
							_stage.ezoneTargetMatcher->CalcXform(tNext,xfmEventEnd);
							xfmEventEnd.applyBase(xfmEnd);
						}

						LevelPos posTarget;
						if (_target.tp!=LevelSkillTarget::Target_FixPosAndObj)
							LevelUtil_CalcTargetPos(level,_target,posTarget);
						else
							posTarget=_target.Pos();

						if (bValidTargetAligner)
						{
							i_math::vector2df dirAligner=LevelFaceToDir(faceTargetAlignerAdjusted);

							float distNext=(xfmNext.pos.getXZ()-xfm.pos.getXZ()).dotProduct(dirAligner);
							float distEnd=(xfmEnd.pos.getXZ()-xfm.pos.getXZ()).dotProduct(dirAligner);
							float distTarget=(posTarget-xfm.pos.getXZ()).dotProduct(dirAligner);

							float distAdjust;
							if (distEnd>0.01f)
								distAdjust=distNext*distTarget/distEnd;
							else
								distAdjust=distTarget;

							distAdjust=i_math::clamp_f(distAdjust,distNext*paramStage->targetmatch.scaleSpeedMin,distNext*paramStage->targetmatch.scaleSpeedMax);

							i_math::vector3df dirAligner3D;
							dirAligner3D.setXZ(dirAligner);
							posDelta+=dirAligner3D*(distAdjust-distNext);
						}
					}
										
				}

			}


			//计算posDelta,faceNext,faceTargetAlignerNext
			if (paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_AnimDriven_Default_Old||
				paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_AnimDriven_RotateOnSpot||
				paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_CodeDriven_MoveAlong)
			{
				LevelFace faceOff=0.0f;//face和target aligner朝向的offset
				LevelFace faceDelta=0.0f;

				if ((paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_AnimDriven_Default_Old)||
					(paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_AnimDriven_RotateOnSpot))
				{
					//动画内的path朝向
					LevelFace faceTargetAligner=LevelFaceFromDir(i_math::vector2df(0.0f,1.0f));//前方
					if (_stage.path)
						LevelFaceApplyYaw(faceTargetAligner,_stage.path->facePath*i_math::GRAD_PI2);

					//计算出的posDelta在path朝向的旋转空间内
					if (TRUE)
					{
						posDelta=xfmAnimNext.pos-_xfmAnim.pos;

						//将posDelta转化到这个动画的target aligner朝向的旋转空间内
						if (TRUE)
						{
							i_math::quatf rot;
							LevelFaceToQuat(faceTargetAligner,rot);
							rot.makeInverse();
							posDelta=rot*posDelta;
						}
					}

					LevelFace face=LevelFaceFromQuat(xfmAnimNext.rot);
					LevelFace faceLast=LevelFaceFromQuat(_xfmAnim.rot);
					faceDelta=i_math::normalize_radian(face-faceLast);
					faceOff=i_math::normalize_radian(face-faceTargetAligner);
				}
				if (paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_CodeDriven_MoveAlong)
					posDelta.z=paramStage->speedMoveAlong*ANIMTICK_TO_SECOND(dt);

				if ((paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_AnimDriven_Default_Old)||
					(paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_CodeDriven_MoveAlong))
				{
					//修正_faceTargetAligner
					faceTargetAlignerNext=_AdjustFacing(_faceTargetAligner,_modePathFacing,paramStage->speedMaxPathFacingAdjust,dt,_yawPathFacing);

					if (_modeFacing!=LevelSkillTargetFacingMode_None)
						faceNext=_AdjustFacing(_face,_modeFacing,paramStage->speedMaxFacingAdjust,dt);
					else
					{
						LevelFace faceNew=faceTargetAlignerNext+faceOff;
						if (FALSE)
						{
							LevelFace limit=paramStage->speedMaxFacingAdjust*i_math::GRAD_PI2*ANIMTICK_TO_SECOND(dt);
							faceNext=_face;
							i_math::rotate_limited(faceNext,faceNew,limit);
						}
						else
							faceNext=faceNew;
					}
				}
				if (paramStage->modeWork==SkillParam_GeneralAdvS::Stage::WorkingMode_AnimDriven_RotateOnSpot)
				{
					faceTargetAlignerNext=_faceTargetAligner;//不变
					faceNext=_face+faceDelta*_stage.scaleFaceRotate;
				}

				if (_bTargetMatching&&(paramStage->modeWork!=SkillParam_GeneralAdvS::Stage::WorkingMode_CodeDriven_MoveAlong))
				{
					LevelPos posTarget;
					if (_target.tp!=LevelSkillTarget::Target_FixPosAndObj)
						LevelUtil_CalcTargetPos(level,_target,posTarget);
					else
						posTarget=_target.Pos();

					LevelPos dirTargetAligner=LevelFaceToDir(faceTargetAlignerNext);
					float distTarget=(posTarget-_stage.posStageStart).dotProduct(dirTargetAligner)+paramStage->targetmatchOld.distOff;
					float dist=(_pos-_stage.posStageStart).dotProduct(dirTargetAligner);

					AnimTick durStage=_stage.dur;
					if (durStage<=0)
						durStage=1;
					AnimTick tLast=ANIMTICK_SAFE_MINUS(t,dt);
					float r=(float)(ANIMTICK_SAFE_MINUS(tLast,_stage.tStart))/(float)durStage;
					r=i_math::clamp_f(r,0.0f,1.0f);

					float distToGo=distTarget-dist;
					float distToGoAnim=0.0f;
					if (_stage.path)
					{
						Key_pos kCur,kEnd;
						_stage.path->ksPos3D.CalcKey(durStage,&kEnd);
						_stage.path->ksPos3D.CalcKey(ANIMTICK_SAFE_MINUS(tLast,_stage.tStart),&kCur);
						distToGoAnim=kEnd.v.getDistanceXZFrom(kCur.v);
						distToGoAnim*=_owner->GetModelScale();
					}

					float distDelta=0.0f;
					if (distToGoAnim>0.001f)
					{
						distDelta=posDelta.z*i_math::clamp_f(distToGo/distToGoAnim,
												paramStage->targetmatchOld.scaleSpeedMin,
												paramStage->targetmatchOld.scaleSpeedMax);
					}

					posDelta.z=distDelta;
				}

				//将posDelta转到target aligner的朝向上
				if (TRUE)
				{
					i_math::xformf xfmPathRot;
					LevelFaceToQuat(faceTargetAlignerNext,xfmPathRot.rot);

					i_math::matrix43f matPathRot;
					xfmPathRot.getMatrix(matPathRot);
					matPathRot.transformVect(posDelta,posDelta);
				}
			}


			//Apply the step(pos)
			if (_methodObstacle==SkillParam_GeneralAdvS::ObstacleMethod_AvoidPhysObstacle)
			{
				CGameTrisMap *gtr=level->GetGtr();

				i_math::vector3df pos3D;
				pos3D=LevelUtil_GetGroundHeight(level,_pos.x,_pos.y,TRUE);
				pos3D.y+=_ht;

				i_math::vector3df eulerBase=posDelta;
				float distDelta=posDelta.getLength();
				eulerBase.normalize();
				eulerBase.toEuler();

				SkillParam_GeneralAdvS::PhysObstacleAvoidDir dirAvoid=param->dirAvoid;
				if (_dirPhysObstacleAvoid!=SkillParam_GeneralAdvS::PhysObstacleAvoidDir_Undefined)
					dirAvoid=_dirPhysObstacleAvoid;

				const float stepOff=10.0f;
				for (float off=0.0f;off<360.0f;off+=stepOff)
				{
					i_math::vector3df dir;
					dir=eulerBase;

					if (dirAvoid==SkillParam_GeneralAdvS::PhysObstacleAvoidDir_CCW)
						dir.x-=off*i_math::GRAD_PI2;
					if (dirAvoid==SkillParam_GeneralAdvS::PhysObstacleAvoidDir_CW)
						dir.x+=off*i_math::GRAD_PI2;

					dir.eulerToDir();

					i_math::vector3df posHit;
					if (!gtr->RayCheck(pos3D,pos3D+dir*param->distAvoid,posHit))
					{
						posDelta=dir*distDelta;
						break;
					}
				}
			}

			if ((_methodObstacle==SkillParam_GeneralAdvS::ObstacleMethod_StopAtStaticObstacleOrEnemyObstacle)||
				(_methodObstacle==SkillParam_GeneralAdvS::ObstacleMethod_StopAtStaticObstacle)||
				(_methodObstacle==SkillParam_GeneralAdvS::ObstacleMethod_StopAtStaticObstacleOrDynObstacle)||
				(_methodObstacle==SkillParam_GeneralAdvS::ObstacleMethod_StopAtStaticObstacleAndBumpEnemyObstacle))
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
						if (_methodObstacle==SkillParam_GeneralAdvS::ObstacleMethod_StopAtStaticObstacleOrDynObstacle)
							flagDetect=(LevelDetectTargetFlag)(LevelDetectTarget_Enemy|LevelDetectTarget_Native|LevelDetectTarget_Ally|LevelDetectTarget_Neutral|
																		LevelDetectTarget_Unit|LevelDetectTarget_Player|LevelDetectTarget_Ground);
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
								_idRecentDynObstacle=loTarget->GetID();

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
									link.id=level->GenOpLinkID();
									link.t=level->GetT_();

									level->GetDecider()->MakeBuff(LevelOSB(this),loTarget,level->GetRecords()->GetGlobal()->idDefBuff_PB,0,&arg,link);
								}
								if ((_methodObstacle==SkillParam_GeneralAdvS::ObstacleMethod_StopAtStaticObstacleOrEnemyObstacle)||
									(_methodObstacle==SkillParam_GeneralAdvS::ObstacleMethod_StopAtStaticObstacleOrDynObstacle))
								{
									step.end=step.start+vRay*distHit;
									dist=distHit;
									// 									level->GetDbgDraw().DrawCircle(step.start,radiusSrc,RGB(0,255,255),2.0f);
									// 									level->GetDbgDraw().DrawCircle(posTarget,radiusTarget,RGB(255,0,255),2.0f);
									// 									level->GetDbgDraw().DrawCircle(step.end,0.03f,RGB(255,0,0),2.0f);
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

					float y=posDelta.y;
					posDelta.setLength(dist3D);
					posDelta.y=y;
				}
			}

			//计算posNext
			posNext=_pos+posDelta.getXZ();

			//计算htNext
			switch(paramStage->heightadjust.mode)
			{
				case SkillParam_GeneralAdvS::Stage::HeightAdjust::Default:
				{
					htNext=_ht+posDelta.y*_scaleHt;
					break;
				}
				case SkillParam_GeneralAdvS::Stage::HeightAdjust::BlendToTargetOverStage:
				{
					AnimTick tLocal=ANIMTICK_SAFE_MINUS(t,_stage.tStart);

					float fT=i_math::clamp_f( (((float)tLocal)/(float)_stage.dur),0.0f,1.0f);

					float r=paramStage->heightadjust.blend.GetFloat(ANIMTICK_FROM_SECOND(fT));

					htNext=i_math::lerp(_stage.htStart,paramStage->heightadjust.htTarget,r);
					break;
				}
				case SkillParam_GeneralAdvS::Stage::HeightAdjust::BlendToTargetOverDur:
				{
					AnimTick tLocal=ANIMTICK_SAFE_MINUS(t,_stage.tStart);

					float fT=i_math::clamp_f( (((float)tLocal)/(float)paramStage->heightadjust.durBlend),0.0f,1.0f);

					float r=paramStage->heightadjust.blend.GetFloat(ANIMTICK_FROM_SECOND(fT));

					htNext=i_math::lerp(_stage.htStart,paramStage->heightadjust.htTarget,r);
					break;
				}
			}
		}

		if (_ges)
			_ges->UpdateLoc(posNext,htNext,faceNext,dt);

		_faceTargetAligner=faceTargetAlignerNext;
		_pos=posNext;
		_face=faceNext;
		_ht=htNext;

		_xfmAnim=xfmAnimNext;
		_tXfmAnim=t;
	}
}

BOOL Skill_GeneralAdvS::CheckInStage(StringID nmStage)
{
	if (_stage.param)
	{
		if (_stage.param->nm==nmStage)
			return TRUE;
	}
	return FALSE;
}

StringID Skill_GeneralAdvS::GetStageNameID()
{
	if (_stage.param)
		return _stage.param->nm;
	return StringID_Invalid;
}

SkillParam_GeneralAdvS::Window *Skill_GeneralAdvS::_FindWindowParam(StringID nmWindow)
{
	SkillParam_GeneralAdvS*param=_rec->GetParam<SkillParam_GeneralAdvS>();
	if (param)
	{
		for (int i=0;i<param->windows.size();i++)
		{
			if (param->windows[i].nm==nmWindow)
				return &param->windows[i];
		}
	}
	return NULL;
}


void Skill_GeneralAdvS::_OpenWindow(StringID nmWindow)
{
	std::unordered_set<StringID>::iterator it=_windows.find(nmWindow);
	if (it==_windows.end())
	{
		_windows.insert(nmWindow);

		SkillParam_GeneralAdvS::Window *paramWindow=_FindWindowParam(nmWindow);
		if (paramWindow)
		{
			if (paramWindow->idOpenEo!=RecordID_Invalid)
			{
				i_math::xformf xfm;
				xfm.pos=_owner->GetFramePos3D();
				LevelFaceToQuat(_owner->GetFrameFace(),xfm.rot);

				CSkillGeneralEvents::CreateEo(paramWindow->idOpenEo,this,xfm,NULL,_tCasting);
			}
		}
	}
}

void Skill_GeneralAdvS::_CloseWindow(StringID nmWindow)
{
	std::unordered_set<StringID>::iterator it=_windows.find(nmWindow);
	if (it!=_windows.end())
	{
		_windows.erase(it);

		SkillParam_GeneralAdvS::Window *paramWindow=_FindWindowParam(nmWindow);
		if (paramWindow)
		{
			if (paramWindow->idCloseEo!=RecordID_Invalid)
			{
				i_math::xformf xfm;
				xfm.pos=_owner->GetFramePos3D();
				LevelFaceToQuat(_owner->GetFrameFace(),xfm.rot);

				CSkillGeneralEvents::CreateEo(paramWindow->idCloseEo,this,xfm,NULL,_tCasting);
			}
		}
	}
}

void Skill_GeneralAdvS::_CloseAllWindows()
{
	std::unordered_set<StringID>::iterator it;
	for (it=_windows.begin();it!=_windows.end();it++)
	{
		StringID nmWindow=(*it);

		SkillParam_GeneralAdvS::Window *paramWindow=_FindWindowParam(nmWindow);
		if (paramWindow)
		{
			if (paramWindow->idCloseEo!=RecordID_Invalid)
			{
				i_math::xformf xfm;
				xfm.pos=_owner->GetFramePos3D();
				LevelFaceToQuat(_owner->GetFrameFace(),xfm.rot);

				CSkillGeneralEvents::CreateEo(paramWindow->idCloseEo,this,xfm,NULL,_tCasting);
			}
		}
	}
	_windows.clear();
}

void Skill_GeneralAdvS::_CallBehaviorRelay(StringID nmRelay,LevelOpLink *link)
{
	CLevelBehavior *behavior=_owner->GetBehaviorAI();
	if (behavior)
		behavior->StartRelay(nmRelay);
}
