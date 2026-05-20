
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoChainedHammer.h"

#include "LevelRecords.h"

#include "LevelOSB.h"
#include "LevelDecider.h"

#include "LevelSensor.h"

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorMem.h"


//////////////////////////////////////////////////////////////////////////
//CChainedHammer
void CChainedHammer::BuildSwingSpline(LevelPos3D &posInitial,LevelFace faceInitial,LevelFace faceOwner,ValueSet &vsRadius,CCubicSpline &spline)
{
	spline.Reset(FALSE);

	if (TRUE)
	{
		int c=9;

		LevelFace face=faceOwner;
		LevelFace faceDelta=faceInitial-faceOwner;
		faceDelta=i_math::normalize_radian(faceDelta);

		LevelFace faceStart=faceOwner+faceDelta;
		LevelFace faceEnd=faceStart+(faceOwner-faceStart)*2.0f;


		LevelFace step=(faceEnd-faceStart)/(float)(c-1);

		i_math::quatf rot;
		for (int i=0;i<c;i++)
		{
			LevelFace face=faceStart+step*(float)i;
			LevelPos dir=LevelFaceToDir(face);

			float r=(float)i/(float)(c-1);

			float radius=vsRadius.GetFloat(ANIMTICK_FROM_SECOND(r));

			LevelPos3D pos;
			pos.setXZ(posInitial.getXZ()+dir*radius);
			pos.y=posInitial.y;
			spline.AddNode(pos,rot);
		}

		spline.BuildSNS();
	}
}


void CChainedHammer::Init(EoChainedHammer *owner,LevelPos3D &src,i_math::vector3df&dir,EoParamChainedHammer *param)
{
	_owner=owner;

	float distIgnoreStatic=param->distIgnoreStatic;
	if (param->distIgnoreStatic>0.0f)
	{//换算成3D空间的距离
		i_math::vector3df dirVer;
		dirVer.set(dir.x,0.0f,dir.z);
		if (dirVer.getLengthSQ()>0.001f)
		{//足够斜
			dirVer.normalize();
			float rate=dirVer.dotProduct(dir);
			distIgnoreStatic/=rate;
		}
	}

	CBulletBase::Init(_owner,param,src,param->radius,param->fall,distIgnoreStatic,param->bMH);
	_param=param;
	_dir=dir;
	_t=0.0f;

	if (param->_mode==EoParamChainedHammer::Mode_Throw)
	{
		_posLocal.set(0,0,0);
		_speed=param->speed;
		//调整速度以保证抛物线命中目标
		if (param->g>0.0f)
			_speed=_AdjustThrowSpeed(owner,src,dir,_param->speed,_param->speedAdj,_param->g,0);
	}

	if (param->_mode==EoParamChainedHammer::Mode_Swing)
	{
		LevelFace faceInitial=LevelFaceFromDir(_dir.getXZ());
		LevelFace faceOwner=faceInitial;
		if (TRUE)
		{
			CLevelObj *loOwner=LevelUtil_GetAliveLo(_owner->GetLevel(),_owner->GetRootOwnerID());
			if (loOwner)
				faceOwner=loOwner->GetFrameFace();
		}
		_faceSwingInitial=faceInitial;
		_faceSwing=faceOwner;
		BuildSwingSpline(src,faceInitial,faceOwner,_param->_vsSwingRadius,_splineSwing);
	}
}

LevelObjID CChainedHammer::_DetectHit_ShieldAmulet(i_math::line3df &line)
{
	if (_owner)
	{
		LevelEoDetectHitArg argHit;
		argHit.radius=_param->radius;
		argHit.fall=_param->fall;
		argHit.idIgnore=_param->bIgnoreHost?((EoChainedHammer*)_owner)->GetHost():LevelObjID_Invalid;

		CLevelObj *hit=((EoChainedHammer*)_owner)->DetectHit_ShieldAmulet(line,argHit);
		if (hit)
			return hit->GetID();
	}
	return LevelObjID_Invalid;
}


LevelObjID CChainedHammer::_DetectHit(i_math::line3df &line)
{
	if (_owner)
	{
		LevelEoDetectHitArg argHit;
		argHit.radius=_param->radius;
		argHit.fall=_param->fall;
		argHit.idIgnore=_param->bIgnoreHost?((EoChainedHammer*)_owner)->GetHost():LevelObjID_Invalid;

		CLevelObj *hit=((EoChainedHammer*)_owner)->DetectHit(line,argHit);
		if (hit)
			return hit->GetID();
	}
	return LevelObjID_Invalid;
}

void CChainedHammer::_DetectHits(i_math::line3df &line,LevelObjHits &hits,CLevelObjHistory &history)
{
	hits.Zero();
	if (_owner)
	{
		LevelEoDetectHitArg argHit;
		argHit.radius=_param->radius;
		argHit.fall=_param->fall;
		argHit.idIgnore=_param->bIgnoreHost?((EoChainedHammer*)_owner)->GetHost():LevelObjID_Invalid;

		((EoChainedHammer*)_owner)->DetectHits(line,argHit,hits,history);
	}
}


extern LevelPos3D CalcBulletPos(LevelPos3D &velInitial,float t,float g);

void CChainedHammer::_UpdateStep(AnimSecond dt,LevelPos3D &dir,float &dDist)
{
	if (_param->_mode==EoParamChainedHammer::Mode_Throw)
	{
		if (_param->g<=0.0f)
		{
			dir=_dir;
			dDist=((float)dt)*_speed;
		}
		else
		{
			_t+=(float)dt;
			LevelPos3D pos=CalcBulletPos(_dir*_speed,_t,_param->g);
			dir=pos-_posLocal;
			dDist=dir.getLength();
			if (dDist>0.0f)
				dir/=dDist;
			_posLocal=pos;
		}
	}

	if (_param->_mode==EoParamChainedHammer::Mode_Swing)
	{
		_t+=(float)dt;

		float r=_t/ANIMTICK_TO_SECOND(_param->_durSwing);
		r=i_math::clamp_f(r,0.0f,1.0f);

		LevelPos3D pos=_splineSwing.GetPosition(r);
		dir=pos-_pos;
		dDist=dir.getLength();
		if (dDist>0.0f)
			dir/=dDist;
		_pos=pos;
	}

}



//////////////////////////////////////////////////////////////////////////
//EoChainedHammer
BIND_EOPARAM(EoChainedHammer,EoParamChainedHammer);
CBulletBase *EoChainedHammer::_CreateBullet()
{
	CChainedHammer *bullet=Class_New2(CChainedHammer);
	EoParamChainedHammer*param=GetParam<EoParamChainedHammer>();
	bullet->Init(this,_GetInitialPos3D(),_GetInitialDir3D(),param);
	if (param->dealsStaticHit.size()>0)
		bullet->SetHiResoStaticCheck(TRUE);
	return bullet;
}

void EoChainedHammer::_DestroyBullet(CBulletBase *bullet0)
{
	CChainedHammer *bullet=(CChainedHammer *)bullet0;
	Safe_Class_Delete(bullet);
}


void EoChainedHammer::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	__super::_OnWriteFirstSync(bp,bContent,idPlayer);

	EoParamChainedHammer*param=GetParam<EoParamChainedHammer>();

	LevelPos3D dirInitial=_GetInitialDir3D();
	bp->Data_WriteSimpleR(dirInitial);
	if (param->_mode==EoParamChainedHammer::Mode_Throw)
		bp->Data_WriteSimple(((CChainedHammer*)_core)->GetSpeed());
	if (param->_mode==EoParamChainedHammer::Mode_Swing)
	{
		LevelFace faceInitial,face;
		((CChainedHammer*)_core)->GetSwingFace(faceInitial,face);
		bp->Data_WriteSimple(faceInitial);
		bp->Data_WriteSimple(face);
	}
	bp->Data_WriteSimple(_idOwner);
	bContent=TRUE;
}

void EoChainedHammer::_OnDetroy()
{
	EoParamChainedHammer*param=GetParam<EoParamChainedHammer>();

	if (TRUE)
	{
		CLevelObj *lo=NULL;
		if (_idOwner!=LevelObjID_Invalid)
			lo=LevelUtil_GetAliveLo(_level,_idOwner);

		if (lo)
		{
			CLevelBehavior *bhv=lo->GetBehaviorAI();
			if (bhv)
			{
				CBehaviorMem *mem=bhv->GetMem(0);
				if (mem)
				{
					if (param->_nmBhvVar)
						mem->SetID(param->_nmBhvVar,BehaviorMemType_ObjID,LevelObjID_Invalid);
					if (param->_nmBhvVarImposter)
						mem->SetID(param->_nmBhvVarImposter,BehaviorMemType_ObjID,LevelObjID_Invalid);
				}
			}
		}
	}

	if (_idImposter!=LevelObjID_Invalid)
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,_idImposter);
		if (lo)
			lo->DeferDestroy();
		_idImposter=LevelObjID_Invalid;
	}



	EoBulletBase::_OnDetroy();
}


void EoChainedHammer::_OnPostCreate()
{
	EoBulletBase::_OnPostCreate();

	EoParamChainedHammer*param=GetParam<EoParamChainedHammer>();

	CLevelObj *lo=NULL;
	_idOwner=GetRootOwnerID();
	if (_idOwner!=LevelObjID_Invalid)
		lo=LevelUtil_GetAliveLo(_level,_idOwner);

	if (param->_nmBhvVar)
	{
		if (lo)
		{
			//把自己记录到owner的行为树变量中
			CLevelBehavior *bhv=lo->GetBehaviorAI();
			if (bhv)
			{
				CBehaviorMem *mem=bhv->GetMem(0);
				if (mem)
					mem->SetID(param->_nmBhvVar,BehaviorMemType_ObjID,GetID());
			}

			//得到当前owner的threat
			if (TRUE)
			{
				CLevelSensor *sensor=lo->GetSensor();
				CLevelObj *loThreat=sensor->GetThreat();
				if (loThreat)
					_idThreat=loThreat->GetID();
			}
		}
	}

	if (lo)
		_posSrc=lo->GetFramePos3D();

	if (TRUE)
	{
		CLevelSkill *skill=GetRootSkill();
		if (skill)
		{
			extern BOOL LevelUtil_CalcTargetPos3D(CLevel *level,LevelSkillTarget &target,LevelPos3D &pos3D);
			LevelUtil_CalcTargetPos3D(GetLevel(),skill->GetTarget(),_posTarget);
		}
		else
		{
			//一般不可能执行到这里,以防万一赋一个值
			_posTarget=_posSrc+_GetInitialDir3D()*10.0f;
		}
	}

	_SetStage(Stage_Throwing);
}


BOOL EoChainedHammer::Break()
{
	EoParamChainedHammer*param=GetParam<EoParamChainedHammer>();

	CLevelObj *loOwner=LevelUtil_GetAliveLo(_level,_idOwner);
	if (!loOwner)
		return TRUE;

	if (!((_stage==Stage_Stuck)||(_stage==Stage_Pulling)))
		return FALSE;

	CLevelDecider *decider=_level->GetDecider();
	CLevelDecider::MakeSkillStunContext *ctxSkillStun=decider->GetSkillStunContext();

	_AddOp(LevelOp_ChainedHammer::Broken,0,ctxSkillStun?ctxSkillStun->link:NULL);
	_SetStage(Stage_Detached);

	if (ctxSkillStun)
	{
		if (ctxSkillStun->osbSrc)
		{
			if (param->_dealsBroken.size()>0)
			{
				DealArg arg;
				if (ctxSkillStun->strike)
					arg.dir.setXZ(ctxSkillStun->strike->GetDir());

				if (ctxSkillStun->link)
					arg.link=*ctxSkillStun->link;

				MakeDeals(param->_dealsBroken,*ctxSkillStun->osbSrc,loOwner,arg,NULL);
			}
		}
	}

	//创建替身
	if (TRUE)
	{
		CLoUnit* lo=NULL;
		lo=(CLoUnit*)_level->CreateObj(Class_Ptr2(CLoUnit));
		lo->PostCreate(LevelPlayerID_PlayerWild,NULL,param->_idImposter,1,NULL,EquipSetPick_None,GetFramePos(),0.0f);
		_level->AddToActives(lo);

		_idImposter=lo->GetID();

		SAFE_RELEASE(lo);
	}

	//记录替身
	if (loOwner&&param->_nmBhvVarImposter!=StringID_Invalid)
	{
		//把替身记录到owner的行为树变量中
		CLevelBehavior *bhv=loOwner->GetBehaviorAI();
		if (bhv)
		{
			CBehaviorMem *mem=bhv->GetMem(0);
			if (mem)
				mem->SetID(param->_nmBhvVarImposter,BehaviorMemType_ObjID,_idImposter);
		}
	}

	return TRUE;
}



void EoChainedHammer::_OnUpdate()
{
	EoParamChainedHammer*param=GetParam<EoParamChainedHammer>();

	if (_stage==Stage_Detached)
	{
		if (_GetT()>=_tStageStart+ANIMTICK_FROM_SECOND(2.0f))
		{
			BOOL LevelUtil_CheckDead(CLevel *level,LevelObjID id);
			if (LevelUtil_CheckDead(_level,_idOwner))
			{
				DeferDestroy();
				return;
			}
		}
	}

	if (_stage==Stage_Grabbing)
	{
		if (_GetT()>=_tStageStart+ANIMTICK_FROM_SECOND(GRAB_DUR))
			DeferDestroy();
	}

	if (_stage==Stage_Withdrawn)
	{
		if (_GetT()>=_tStageStart+ANIMTICK_FROM_SECOND(0.2f))
			DeferDestroy();
	}

	if (_stage==Stage_Withdrawing)
	{
		if (_GetT()>=_tStageStart+_durWithdraw)
			_SetStage(Stage_Withdrawn);
	}

	if (_stage==Stage_Pulling)
	{
		if (_GetT()>=_tStageStart+param->_durPullOut)
		{
			_SetStage(Stage_Withdrawing);

			CLevelObj *loOwner=LevelUtil_GetAliveLo(_level,_idOwner);
			if (loOwner)
			{
				float dist=GetFramePos().getDistanceFrom(loOwner->GetFramePos());
				_durWithdraw=ANIMTICK_FROM_SECOND(dist/param->_spdWithdraw);

				_AddOp(LevelOp_ChainedHammer::PullOut,_durWithdraw);
			}
		}
	}

	if (_stage==Stage_Hit)
	{
		CLevelSkill *skill=NULL;
		CLevelObj *loOwner=LevelUtil_GetAliveLo(_level,_idOwner);
		if (loOwner)
			skill=LevelUtil_GetCastingSkill(loOwner);

		if (!skill)
			Withdraw();
	}

	if (_stage==Stage_Throwing)
	{
		EoBulletBase::_OnUpdate();
		if (!_hitStaticToSend.IsEmpty())//侦测到有static hit
		{
			if (param->_mode==EoParamChainedHammer::Mode_Throw)
				_SetStage(Stage_Stuck);
			if (param->_mode==EoParamChainedHammer::Mode_Swing)
				Withdraw();
		}
		else
		{
			if (!_hitsToSend.IsEmpty())
			{
				if (param->_mode==EoParamChainedHammer::Mode_Swing)
					Withdraw();
				else
				{
	// 				Withdraw();
					_SetStage(Stage_Hit);
				}
			}
			else
			{
				BOOL bSwingFinish=FALSE;
				if (param->_mode==EoParamChainedHammer::Mode_Swing)
				{
					float r=((CChainedHammer*)_core)->GetT()/ANIMTICK_TO_SECOND(param->_durSwing);
					if (r>=0.8f)
						bSwingFinish=TRUE;
				}
				if (bSwingFinish)
					Withdraw();
				else
				{
					CLevelSkill *skill=NULL;
					CLevelObj *loOwner=LevelUtil_GetAliveLo(_level,_idOwner);
					if (loOwner)
						skill=LevelUtil_GetCastingSkill(loOwner);

					if (!skill)
						Withdraw();
				}
			}
		}
	}
}

LevelPos EoChainedHammer::GetFramePos()
{
	if (_core)
		return _core->GetPos().getXZ();
	return EoBulletBase::GetFramePos();
}

LevelPos3D EoChainedHammer::GetFramePos3D()
{
	if (_core)
		return _core->GetPos();
	return EoBulletBase::GetFramePos3D();
}

void EoChainedHammer::_SetStage(Stage stage)
{
	_stage=stage;
	_tStageStart=_GetT();
}


void EoChainedHammer::_AddOp(LevelOp_ChainedHammer::Op op_,AnimTick dur,LevelOpLink *link0)
{
	LevelOpLink link;
	link.id=link0?link0->id:_level->GenOpLinkID();
	link.t=ANIMTICK_SAFE_MINUS(_GetT(),_tCreate);
	LevelOp_ChainedHammer*op=LevelOSB(this).NewOp<LevelOp_ChainedHammer>(link);
	op->op=op_;
	op->dur=dur;
	AddOp(op);
}


BOOL EoChainedHammer::Withdraw()
{
 	if ((_stage!=Stage_Throwing)&&(_stage!=Stage_Hit))
		return FALSE;

	CLevelObj *loOwner=LevelUtil_GetAliveLo(_level,_idOwner);
	if (!loOwner)
		return FALSE;

	EoParamChainedHammer*param=GetParam<EoParamChainedHammer>();

	_SetStage(Stage_Withdrawing);

	float dist=GetFramePos().getDistanceFrom(loOwner->GetFramePos());
	_durWithdraw=ANIMTICK_FROM_SECOND(dist/param->_spdWithdraw);

	_AddOp(LevelOp_ChainedHammer::Withdraw,_durWithdraw);

	return TRUE;
}

BOOL EoChainedHammer::Grab()
{
	if (_stage!=Stage_Detached)
		return FALSE;

	CLevelObj *loOwner=LevelUtil_GetAliveLo(_level,_idOwner);
	if (!loOwner)
		return FALSE;

	EoParamChainedHammer*param=GetParam<EoParamChainedHammer>();

	_SetStage(Stage_Grabbing);

	_AddOp(LevelOp_ChainedHammer::Grab,ANIMTICK_FROM_SECOND(GRAB_DUR));

	return TRUE;
}


BOOL EoChainedHammer::Pull()
{
	if (_stage!=Stage_Stuck)
		return FALSE;

	CLevelObj *loOwner=LevelUtil_GetAliveLo(_level,_idOwner);
	if (!loOwner)
		return FALSE;

	EoParamChainedHammer*param=GetParam<EoParamChainedHammer>();

	_SetStage(Stage_Pulling);

	_AddOp(LevelOp_ChainedHammer::Pull,param->_durPullOut);

	return TRUE;
}

