/********************************************************************
	created:	2012/10/23 
	author:		cxi
	
	purpose:	AI Actions
*********************************************************************/
#include "stdh.h"
#include "LevelBGs.h"

#include "BgnFlyingHover.h"

#include "Random/Random.h"

#include "LevelSensor.h"


#include "Level.h"

////////////////////////////////////////////////////////////////////////
//CLevelGesture_FlyingHover

AnimTick CLevelGesture_FlyingHover::_GetT()
{
	if (_owner)
		return _owner->GetT();
	return 0;
}


void CLevelGesture_FlyingHover::Init(CLevelObj *lo,CBgp_FlyingHover *bgp,LevelPos &posSpecified,LevelObjID idFacingTarget)
{
	_owner=lo;
	SAFE_ADDREF(_owner);

	_bgp=bgp;

	AnimTick t=_GetT();

	_posCur=_owner->GetFramePos();

	_tStart=t;

	if (bgp->_dur>0)
		_dur=CSysRandom::RandVary(bgp->_dur,bgp->_durVary);
	else
		_dur=0;//永久

	_posInitial=_owner->GetFramePos();
	_posSpecified=posSpecified;

	_idFacingTarget=idFacingTarget;

	_UpdateCenterPos(t);
}

void CLevelGesture_FlyingHover::Destroy()
{
	SAFE_RELEASE(_owner);
	Zero();
	Release();
}

void CLevelGesture_FlyingHover::_UpdateCenterPos(AnimTick t)
{
	switch(_bgp->_modeCenter)
	{
		case CBgp_FlyingHover::HoveringCenter_Random:
			_GenNewPos(t);
			break;
		case CBgp_FlyingHover::HoveringCenter_Master:
			if (!_RecordMasterPos())
				_GenNewPos(t);
			break;
		case CBgp_FlyingHover::HoveringCenter_Threat:
			if (!_RecordThreatPos())
				_GenNewPos(t);
			break;
		case CBgp_FlyingHover::HoveringCenter_CurPos:
		{
			_posCur=_posInitial;
			break;
		}
		case CBgp_FlyingHover::HoveringCenter_SpecifiedPos:
		{
			_posCur=_posSpecified;
			break;
		}
	}
}


void CLevelGesture_FlyingHover::_GenNewPos(AnimTick t)
{
	if (_tNextMove>t)
		return;

	LevelPos pos=_posCur;

	float rangeX=CSysRandom::RandRange(-_bgp->_range,_bgp->_range);
	float rangeY=CSysRandom::RandRange(-_bgp->_range,_bgp->_range);

	pos.x+=rangeX;
	pos.y+=rangeY;

	_posCur=pos;

	AnimTick dur=CSysRandom::RandVaryUInt(_bgp->_gap,_bgp->_gapVary);
	_tNextMove=t+dur;
}

BOOL CLevelGesture_FlyingHover::_RecordMasterPos()
{
	if (_owner)
	{
		LevelPlayerID idPlayer=_owner->GetPlayerID();
		CLevelPlayer *player=_owner->GetLevel()->GetPlayer(idPlayer);
		if (player)
		{
			CLevelObj *lo=(CLevelObj*)player->GetLoUnit();
			if (lo)
			{
				_posCur=lo->GetFramePos();
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CLevelGesture_FlyingHover::_RecordThreatPos()
{
	extern CLevelObj *LevelUtil_GetThreat(CLevelObj *lo);
	CLevelObj *threat=LevelUtil_GetThreat(_owner);
	if (threat)
	{
		_posCur=threat->GetFramePos();
		return TRUE;
	}

	return FALSE;
}


//返回是否结束
void CLevelGesture_FlyingHover::Update(CUnit3D *unit,float dt)
{
	AnimTick t=_GetT();

	if (_dur>0)
	{
		if (t>_tStart+_dur)
			_bFinished=TRUE;
	}

	_UpdateCenterPos(t);

	i_math::vector3df vel,dir;
	float dist,speed=0.0f;
	if (TRUE)
	{
		LevelPos pos=_owner->GetFramePos();

		vel=unit->GetVel();
		speed=unit->GetSpeed();
		if (_bgp->_bSpeedOverride)
			speed=_bgp->_speed;

		LevelPos dirToTargetT=_posCur-pos;
		dir.set(dirToTargetT.x,0.0f,dirToTargetT.y);
		dist=(float)dir.getLength();
		if (dist>0.001f)
			dir/=dist;
		else
		{
			dir.set(1,0,0);
			dist=0.0f;
		}
	}

	BOOL bZeroVel=FALSE;
	if (vel.getLengthSQ()<0.0001f)
		bZeroVel=TRUE;
	i_math::vector3df eulerVel,eulerDir;
	if (!bZeroVel)
	{
		eulerVel=vel;
		eulerVel.normalize();
		eulerVel.toEuler();
		eulerVel.y=eulerVel.z=0.0f;
	}

	eulerDir=dir;
	eulerDir.toEuler();
	eulerDir.y=eulerDir.z=0.0f;

	float rangeHover=_bgp->_rangeHover;
	float wtBack;//返回到target的倾向,1表示全力回到target
	wtBack=i_math::clamp_f((dist)/(rangeHover),0.0f,1.0f);

	float twistMin=i_math::clamp_f(_bgp->_twist-_bgp->_twistVary,0.0f,1.0f);
	float twistMax=i_math::clamp_f(_bgp->_twist+_bgp->_twistVary,0.0f,1.0f);

	float twist=CSysRandom::RandRange(twistMin,twistMax)*i_math::Pi*wtBack;

	// 	//更新偏离值
	// 	float twistActual;
	// 	if (TRUE)
	// 	{
	// 		float twistrange=1.2f*i_math::Pi;
	// 
	// 		int n=(int)(_twist/twistrange);
	// 		if (n%2==0)
	// 			twistActual=_twist-twistrange*(float)n;
	// 		else
	// 			twistActual=twistrange*(float)(n+1)-_twist;
	// 
	// 		twistActual-=twistrange/2.0f;
	// 	}
	// 	twist+=twistActual;
	// 	if ()


	if (bZeroVel)
		eulerVel=eulerDir;
	else
		rotate_limited(eulerVel.x,eulerDir.x,twist);

	i_math::quatf q;
	q.fromEuler(eulerVel);

	i_math::vector3df velToTarget(0,0,1);

	//根据当前的地面的高度,来决定要升高还是降低
	velToTarget=q*velToTarget;

	if(TRUE)
	{
		GameTileMap *gtm=unit->GetGTM();
		float ht=gtm->GetHeight(unit->_pos.x,unit->_pos.z);
		if (unit->_pos.y<ht+_bgp->_verMin)
		{
			velToTarget.y=1.0f;
 			velToTarget.normalize();
		}
		else
		{
			if (unit->_pos.y>ht+_bgp->_verMax)
			{
				velToTarget.y=-1.0f;
 				velToTarget.normalize();
			}
		}
	}

	velToTarget*=speed;

	BOOL bTargetFace=FALSE;
	LevelFace faceTarget;
	if (_bgp->_modeFacing!=CBgp_FlyingHover::Facing_Default)
	{
		if (_bgp->_modeFacing==CBgp_FlyingHover::Facing_Threat)
		{
			extern CLevelObj *LevelUtil_GetThreat(CLevelObj *lo);
			CLevelObj *loThreat=LevelUtil_GetThreat(_owner);
			if (loThreat)
			{
				LevelPos posThreat=loThreat->GetFramePos();
				LevelPos dir=posThreat-_owner->GetFramePos();
				faceTarget=LevelFaceFromDir(dir);
				bTargetFace=TRUE;
			}
		}
		if (_bgp->_modeFacing==CBgp_FlyingHover::Facing_SpecifiedTarget)
		{
			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			CLevelObj *loTarget=LevelUtil_GetAliveLo(_owner->GetLevel(),_idFacingTarget);
			if (loTarget)
			{
				LevelPos dir=loTarget->GetFramePos()-_owner->GetFramePos();
				faceTarget=LevelFaceFromDir(dir);
				bTargetFace=TRUE;
			}
		}
	}

	if (!bTargetFace)
		unit->AccumVelPos(velToTarget,_bgp->_blend,dt);
	else
		unit->AccumVelPosWithFace(velToTarget,_bgp->_blend,faceTarget,dt);
}



////////////////////////////////////////////////////////////////////////
//CBgn_FlyingHover

BIND_BGN_CLASS(CBgn_FlyingHover,CBgp_FlyingHover);


void CBgn_FlyingHover::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_FlyingHover*pad=_GetPad<CBgp_FlyingHover>();

	_ges=Class_New2(CLevelGesture_FlyingHover);
	_ges->AddRef();

	CLevelObj *lo=_GetLo();

	LevelPos posSpecified=lo->GetFramePos();
	if (pad->_nmPos!=StringID_Invalid)
		_GetPos(pad->_nmPos,posSpecified);
	LevelObjID idFacingTarget=LevelObjID_Invalid;
	if (pad->_nmFacingTarget!=StringID_Invalid)
		_GetID(pad->_nmFacingTarget,BehaviorMemType_ObjID,idFacingTarget);
	_ges->Init(lo,pad,posSpecified,idFacingTarget);

	if (lo)
	{
		CUnit3D *unit=lo->GetUnit3D();
		if (unit)
			unit->SetGesture(_ges);
	}
}

//返回是否结束
BOOL CBgn_FlyingHover::_Update()
{
	if (!_ges)
		return TRUE;
	if (!_ges->IsValid())
		return TRUE;

	CBgp_FlyingHover*pad=_GetPad<CBgp_FlyingHover>();
	if (pad->_bTrackPosVar)
	{
		if (pad->_nmPos!=StringID_Invalid)
		{
			LevelPos posSpecified;
			if (_GetPos(pad->_nmPos,posSpecified))
			{
				_ges->SetSpecifiedPos(posSpecified);
			}
		}
	}

	if (_ges->IsFinished())
		return TRUE;
	return FALSE;
}


void CBgn_FlyingHover::Update(BGNOutputs &outputs)
{
	if (_Update())
	{
		_OutputOk(outputs,1,"结束");
		return;
	}
}


void CBgn_FlyingHover::Destroy()
{
	if (_ges)
		_ges->_bFinished=TRUE;
	SAFE_RELEASE(_ges);
}

void CBgn_FlyingHover::Break(BGNOutputs &outputs)
{
	Destroy();
}
