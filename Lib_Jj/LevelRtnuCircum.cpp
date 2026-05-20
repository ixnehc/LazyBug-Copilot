
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "Level.h"
#include "gamedata/GameTileMap.h"

#include "LevelUtil.h"

#include "LevelSensor.h"

#include "LevelRtnuCircum.h"

#include "LoUnit.h"

#include "rvo2/RvoSimulator.h"
#include "rvo2/RvoUnit.h"

//////////////////////////////////////////////////////////////////////////
//CRtnuUnit

void CRtnuUnit::Clear()
{
	CUnit::_Clear();
	Zero();
}

CUnit *CRtnuUnit::GetMirrorUnit()
{
	if (_unitRvo)
		return _unitRvo->getMirror();
	return NULL;
}

UnitStage CRtnuUnit::GetStage()
{
	if (_bRotateOnSpot)
		return UnitStage_RotateOnSpot;
	return __super::GetStage();
}

BOOL CRtnuUnit::IsAvoiding()
{
	if (_unitRvo&&(_unitRvo->getLastAvoidRad()>=0.0f))
		return TRUE;
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelRtnuCircum
void CLevelRtnuCircum::Init(CLevelPlayer *player)
{
	_owner=player;
	_level=player->GetLevel();
	_unitmgr=_level->GetUnitMgr();

}

void CLevelRtnuCircum::Clear()
{
	RvoSimulator &sim=_level->GetRvoSim();

	if (_unitRvoOwner)
		sim.removeUnit(_unitRvoOwner);

	for (int i=0;i<ARRAY_SIZE(_units);i++)
	{
		for (int j=0;j<_units[i].size();j++)
		{
			CRtnuUnit *unitRtnu=_units[i][j];
			if (unitRtnu)
				sim.removeUnit(unitRtnu->_unitRvo);

			unitRtnu->Clear();
			SAFE_RELEASE(unitRtnu);
		}

		_units[i].clear();
	}

	Zero();
}


CRtnuUnit *CLevelRtnuCircum::Register(LevelRtnuRank rank,CLevelObj *lo,LevelPos &pos,LevelFace face)
{
	if (rank>=LevelRtnuRank_Max)
		return NULL;

	RvoSimulator &sim=_level->GetRvoSim();

	CRtnuUnit *unitRtnu=Class_New2(CRtnuUnit);

	unitRtnu->_rank=rank;
	unitRtnu->SetMgr(_unitmgr);
	unitRtnu->Reset(pos,face);
	unitRtnu->SetRadius(lo->GetRadius_());

	unitRtnu->_status=CRtnuUnit::Status_Accompanying;

	unitRtnu->SetAlive();
	unitRtnu->AddRef();

	unitRtnu->_faceCur=face;
	unitRtnu->SetData((void*)lo);

	if (TRUE)
	{
		CLevelSensor *sensor=lo->GetSensor();
		if (sensor)
			sensor->BeginOverrideThreat();
	}

	unitRtnu->_unitRvo=sim.addUnit(unitRtnu->_pos,10.0f,10,0.04f,sim.getTimeStep()*0.5f,unitRtnu->_radius,0.0f);
	unitRtnu->_unitRvo->setWeight(1.0f);

	_units[rank].push_back(unitRtnu);

	return unitRtnu;
}

void CLevelRtnuCircum::Unregister(CRtnuUnit *unitRtnu)
{
	if (!unitRtnu)
		return;

	CLevelObj *lo=(CLevelObj *)unitRtnu->GetData();
	if (TRUE)
	{
		CLevelSensor *sensor=lo->GetSensor();
		if (sensor)
			sensor->EndOverrideThreat();
	}


	RvoSimulator &sim=_level->GetRvoSim();
	sim.removeUnit(unitRtnu->_unitRvo);

	std::vector<CRtnuUnit*>&buf=_units[unitRtnu->_rank];
	VEC_REMOVE_SWAP(buf,unitRtnu);

	unitRtnu->Clear();
	SAFE_RELEASE(unitRtnu);
}


BOOL FindCircumPos_Knight(LevelPos &pos,LevelPos &posTarget,LevelMoveStep &stepOwner)
{
	if (stepOwner.bReaching)
		return FALSE;
	// 	if (stepOwner.dist<=0.0f)
	// 		return FALSE;	

	const float range=3.0f;
	const float rangeOutter=5.0f;
	const float fov=180.0f;
	const float d=cosf(fov*i_math::GRAD_PI2/2.0f);

	LevelPos posOwnerTarget=stepOwner.pos+stepOwner.dir*stepOwner.dist;
	posTarget=pos;

	BOOL bInFov=FALSE;
	if ((pos-stepOwner.pos).dotProduct(stepOwner.dir)>=d)
		bInFov=TRUE;

	BOOL bInRange=FALSE;
	if (pos.getDistanceSQFrom(stepOwner.pos)<range*range)
		bInRange=TRUE;

	if (bInRange&&bInFov)
		posTarget=pos+stepOwner.dir*stepOwner.dist;
	else
	{
		if (!bInRange)
		{
			// 			LevelPos dir=posOwnerTarget-pos;
			// 			dir.safe_normalize();
			// 
			// 			if (!bInFov)
			// 				dir*=stepOwner.dist*1.2f;
			// 
			// 			posTarget=pos+dir;
			posTarget=posOwnerTarget;
		}
		else
		{
			//InRange but not InFov
			posTarget=pos+stepOwner.dir*stepOwner.dist*1.2f;
		}

	}

	return TRUE;
}

BOOL FindCircumPos_Knight2(LevelPos &pos,CLevelObj *loThreat,LevelMoveStep &stepOwner,float dt,LevelPos &posTargetRet,float &speedRet)
{
	// 	if (stepOwner.bReaching)
	// 		return FALSE;

	LevelPos dirConstraint=stepOwner.GetFinalPos()-pos;
	float distToOwner=dirConstraint.getLength();
	dirConstraint.safe_normalize();

	const float distConstraint=2.0f;
	LevelPos velH=stepOwner.dir*stepOwner.speed*1.1f;
	if (stepOwner.bReaching)
		velH.set(0,0);
	LevelPos velConstraint=dirConstraint*pow(distToOwner/distConstraint,1.5f);

	LevelPos vel=velH+velConstraint;

	if (TRUE)
	{
		LevelPos dir=vel;
		dir.safe_normalize();
		speedRet=vel.getLength();
		speedRet=i_math::clamp_f(speedRet,stepOwner.speed*0.8f,stepOwner.speed*1.4f);
		vel=dir*speedRet;
	}

	posTargetRet=pos+vel*dt;

	return TRUE;
}

BOOL FindCircumPos_Knight3(LevelPos &pos,CLevelObj *loThreat,LevelMoveStep &stepOwner,float dt,LevelPos &posTargetRet,float &speedRet)
{
	LevelPos posGuide;
	if (TRUE)
	{
		LevelPos dir;
		if (!loThreat)
			dir=pos-stepOwner.pos;
		else
			dir=loThreat->GetFramePos()-stepOwner.pos;

		float distToOwner=pos.getDistanceFrom(stepOwner.pos);
		dir.safe_normalize();

		LevelPos posOwner=stepOwner.GetFinalPos();

		LevelFace faceOwner=LevelFaceFromDir(stepOwner.dir);

		LevelFace face=LevelFaceFromDir(dir);

		face-=faceOwner;
		face=i_math::normalize_radian(face);

		if (!loThreat)
		{
			//clamp到faceOwner的后方
			if (face>0.0f)
			{
				if (face<i_math::Pi/3.0f)
					face=i_math::Pi/3.0f;
			}
			else
			{
				if (face>-i_math::Pi/3.0f)
					face=-i_math::Pi/3.0f;
			}
		}
		else
		{
			//clamp到Threat方向的一个扇形
		}

		face+=faceOwner;

		dir=LevelFaceToDir(face);

		posGuide=posOwner+dir*1.0f;//distToOwner*0.8f;
	}

	LevelPos velH=stepOwner.dir*stepOwner.speed;
	if (stepOwner.bReaching)
		velH.set(0,0);
	else
	{
		if (loThreat)
		{

		}
	}

	LevelPos velConstraint;
	if (TRUE)
	{
		LevelPos dirGuide=posGuide-pos;
		float distToGuide=dirGuide.getLength();
//		dirGuide.safe_normalize();

		const float distConstraint=2.0f;
		velConstraint=dirGuide/dt;
		velConstraint=velConstraint*pow(distToGuide/distConstraint,1.5f);
	}

	LevelPos vel=velH+velConstraint;

	if (TRUE)
	{
		LevelPos dir=vel;
		dir.safe_normalize();
		speedRet=vel.getLength();
		speedRet=i_math::clamp_f(speedRet,stepOwner.speed*0.8f,stepOwner.speed*1.4f);
		vel=dir*speedRet;
	}

	posTargetRet=pos+vel*dt;

	return TRUE;
}

struct Fan
{
	LevelPos center;
	float face;
	float fov;
	float n;
	float f;

	void Clamp(LevelPos &pos)
	{
		LevelPos dir=pos-center;
		LevelFace face2=LevelFaceFromDir(dir);
		float dist=dir.getLength();

		face2-=face;
		i_math::normalize_radian(face2);
		face2=i_math::clamp_f(face2,-fov/2.0f,fov/2.0f);
		face2+=face;

		dist=i_math::clamp_f(dist,n,f);

		pos=center+LevelFaceToDir(face2)*dist;
	}
};

BOOL FindCircumPos_Knight4(LevelPos &pos,CLevelObj *loThreat,LevelMoveStep &stepOwner,float dt,LevelPos &posTargetRet,float &speedRet)
{
	Fan fanConstraint;
	if (loThreat)
	{
		LevelPos dir;
		dir=loThreat->GetFramePos()-stepOwner.pos;
		fanConstraint.face=LevelFaceFromDir(dir);
		fanConstraint.n=0.0f;
		fanConstraint.f=1.0f;
		fanConstraint.fov=240.0f*i_math::GRAD_PI2;
	}
	else
	{
		fanConstraint.face=LevelFaceFromDir(-stepOwner.dir);
		fanConstraint.n=0.0f;
		fanConstraint.f=1.0f;
		fanConstraint.fov=240.0f*i_math::GRAD_PI2;
	}
	fanConstraint.center=stepOwner.GetFinalPos();

	LevelPos posTarget;
	if (!loThreat)
		posTarget=pos+stepOwner.dir*stepOwner.dist;
	else
		posTarget=loThreat->GetFramePos();

	fanConstraint.Clamp(posTarget);


	speedRet=posTarget.getDistanceFrom(pos)/dt;
	speedRet=i_math::clamp_f(speedRet,stepOwner.speed*0.8f,stepOwner.speed*1.4f);

	posTargetRet=posTarget;

	return TRUE;
}

void CLevelRtnuCircum::_StartGathering(CRtnuUnit *unitRtnu,LevelPos &posOwner,float speedGathering,float dt)
{
	unitRtnu->RequestTargetPos(posOwner);
	unitRtnu->SetSpeed(speedGathering);
	unitRtnu->UpdateStage(dt);

	float distToGo=unitRtnu->GetDistToGo();
	if (distToGo<0.0f)
		distToGo=unitRtnu->_pos.getDistanceFrom(posOwner);

	unitRtnu->_durGathering=(distToGo-1.0f)/speedGathering;

	unitRtnu->_status=CRtnuUnit::Status_Gathering;

}



void CLevelRtnuCircum::_UpdateUnit_Knight(CRtnuUnit *unitRtnu,LevelMoveStep &stepOwnerMove,float dt)
{
	LevelPos posOwner=stepOwnerMove.GetFinalPos();

	CLevelObj *loThreat=NULL;
	CLevelObj *loUnit=(CLevelObj *)unitRtnu->GetData();
	if (loUnit)
	{
		CLevelSensor *sensor=loUnit->GetSensor();
		if (sensor)
			loThreat=sensor->GetThreat();
	}

	//Movement
	if (TRUE)
	{
		BOOL bStopped=FALSE;

		const float speedGathering=stepOwnerMove.speed*0.8f;

		const float speedFollowing=stepOwnerMove.speed*1.5f;
		const float distFollowingMax=7.0f;
		const float distFollowingMin=5.0f;
		const float durFollowing=2.0f;

		if (loUnit->TestBuffFlag(BuffFlag_Pausing))
		{
			unitRtnu->Reset();
			bStopped=TRUE;
		}


// 		if (!bStopped)
// 		{
// 			if (stepOwnerMove.bReaching)
// 			{
// 				if (unitRtnu->GetPos().getDistanceSQFrom(posOwner)<7.0f*7.0f)
// 				{
// 					unitRtnu->StopMove();
// 					bStopped=TRUE;
// 				}
// 			}
// 		}

		if (!bStopped)
		{
			switch(unitRtnu->_status)
			{
				case CRtnuUnit::Status_Accompanying:
				{
					if (stepOwnerMove.bReaching)
						_StartGathering(unitRtnu,posOwner,speedGathering,dt);
					else
					{
						if (unitRtnu->_pos.getDistanceSQFrom(posOwner)>=distFollowingMax*distFollowingMax)
						{
							unitRtnu->_status=CRtnuUnit::Status_Following;
							unitRtnu->_durFollowing=durFollowing;
						}
					}
					break;
				}
				case CRtnuUnit::Status_GatheringAvoiding:
				{
					if (stepOwnerMove.bReaching)
					{
						if (!unitRtnu->IsAvoiding())
							_StartGathering(unitRtnu,posOwner,speedGathering,dt);
					}
					else
						unitRtnu->_status=CRtnuUnit::Status_Accompanying;
					break;
				}
				case CRtnuUnit::Status_Gathering:
				{
					if (stepOwnerMove.bReaching)
					{
						if (unitRtnu->IsAvoiding())
							unitRtnu->_status=CRtnuUnit::Status_GatheringAvoiding;
						else
						{
							if (unitRtnu->_durGathering<=0.0f)
							{
								unitRtnu->StopMove();
								bStopped=TRUE;
							}
							else
								unitRtnu->_durGathering-=dt;
						}
					}
					else
						unitRtnu->_status=CRtnuUnit::Status_Accompanying;
					break;
				}
				case CRtnuUnit::Status_Following:
				{
					if (stepOwnerMove.bReaching)
						_StartGathering(unitRtnu,posOwner,speedGathering,dt);
					else
					{
						if (unitRtnu->_pos.getDistanceSQFrom(posOwner)<distFollowingMin*distFollowingMin)
							unitRtnu->_status=CRtnuUnit::Status_Accompanying;
						else
						{
							if (unitRtnu->_durFollowing<=0.0f)
								unitRtnu->_status=CRtnuUnit::Status_Accompanying;
							else
								unitRtnu->_durFollowing-=dt;
						}
					}

					break;
				}
			}
		}

		if (!bStopped)
		{
			LevelPos posTarget;
			float speed;
			if (unitRtnu->_status==CRtnuUnit::Status_Accompanying)
			{
				if (FindCircumPos_Knight3(unitRtnu->GetPos(),loThreat,stepOwnerMove,dt,posTarget,speed))
	// 			if (FindCircumPos_Knight2(unitRtnu->GetPos(),posTarget,speed,stepOwnerMove,dt))
				{
					if (_unitmgr->StaticObstacleTest(UnitFindPath_Walkable,unitRtnu->GetPos(),posTarget))
						posTarget=posOwner;
				}
			}
			else
			{
				posTarget=posOwner;
				if ((unitRtnu->_status==CRtnuUnit::Status_Gathering)||(unitRtnu->_status==CRtnuUnit::Status_GatheringAvoiding))
					speed=speedGathering;
				else
					speed=speedFollowing;
			}
			unitRtnu->RequestTargetPos(posTarget);
			unitRtnu->SetSpeed(speed);
		}

		unitRtnu->UpdateStage(dt);
		if (unitRtnu->GetStage()==_UnitStage_Blocked)
		{
			unitRtnu->UpdateStage(dt);
			if (unitRtnu->GetStage()==_UnitStage_Blocked)
				unitRtnu->StopMove();
		}
	}

	//Face
	while (TRUE)
	{
		const float spdRot=360.0f*i_math::GRAD_PI2;
		const float radRotateOnSpot=30.0f*i_math::GRAD_PI2;

		//更新_faceOwner
		if (TRUE)
		{
			if (!_stepOwnerMove.bReaching)
			{
				unitRtnu->_faceOwner=LevelFaceFromDir(stepOwnerMove.dir);
				unitRtnu->_wtFaceOwner=1.0f;
			}
			else
			{
				unitRtnu->_wtFaceOwner-=5.0f*dt;
				if (unitRtnu->_wtFaceOwner<0.0f)
					unitRtnu->_wtFaceOwner=0.0f;
			}
		}

		BOOL bMoving=FALSE;
		if (unitRtnu->GetMoveDist()>0.0f)
			bMoving=TRUE;

		if (unitRtnu->_bRotateOnSpot)
		{
			if (!bMoving)
			{
				if (i_math::rotate_limited(unitRtnu->_faceCur,unitRtnu->_faceRotateOnSpot,spdRot*dt))
					unitRtnu->StopRotateOnSpot();
				unitRtnu->SetFace(unitRtnu->_faceCur);
				break;
			}
			//开始移动了,我们立即停止RotateOnSpot
			unitRtnu->StopRotateOnSpot();
		}

		if (loThreat&&unitRtnu->AllowSlideMoving())
		{
			LevelPos dir=loThreat->GetFramePos()-unitRtnu->GetPos();
			LevelFace faceThreat=LevelFaceFromDir(dir);
			if (bMoving)
				i_math::rotate_limited(unitRtnu->_faceCur,faceThreat,spdRot*dt);
			else
			{
				if (get_radian_dist(unitRtnu->_faceCur,faceThreat)>radRotateOnSpot)
					unitRtnu->StartRotateOnSpot(faceThreat);
			}
			unitRtnu->SetFace(unitRtnu->_faceCur);
		}
		else
		{

// 			if (unitRtnu->_status==CRtnuUnit::Status_Following)
// 				unitRtnu->_faceCur=unitRtnu->_face;
// 			else
// 				unitRtnu->_faceCur=LevelFaceFromDir(stepOwnerMove.dir);
		}
		break;
	}
}


void CLevelRtnuCircum::PreSimulate()
{
	if (!_owner)
		return;

	CLoUnit *loOwnerUnit=_owner->GetLoUnit();
	if (!loOwnerUnit)
		return;

	RvoSimulator &sim=_level->GetRvoSim();
	float dt=sim.getTimeStep();

	_UpdateThreats();

	LevelFace faceOwner;
	float speedOwner;
	LevelPos posOwner;
	LevelPos posOwnerPredicted;

	faceOwner=loOwnerUnit->GetFrameFace();
	speedOwner=loOwnerUnit->GetMove()->CalcGroundSpeed();
	_owner->GetMove().GetRecentMoveStep(_level->GetT_(),ANIMTICK_FROM_SECOND(dt),_stepOwnerMove);
	posOwner=_stepOwnerMove.GetFinalPos();
	posOwnerPredicted=_owner->GetMove().CalcPos(_level->GetT_()+ANIMTICK_FROM_SECOND(0.1f));

	if (!_unitRvoOwner)
	{
		_unitRvoOwner=sim.addUnit(posOwner,10.0f,10,1.0f,0.1f,loOwnerUnit->GetRadius_(),0.0f,FALSE);
		_unitRvoOwner->setWeight(1.0f);
	}

	_unitRvoOwner->Pos()=posOwnerPredicted;
	_unitRvoOwner->setPrefVelocity(i_math::vector2df(0.0f,0.0f));
//	_unitRvoOwner->setMaxSpeed(0.0f);

	sim.GetAvoidHintTrail().AddPos(posOwner);

	for (int i=0;i<ARRAY_SIZE(_units);i++)
	{
		for (int j=0;j<_units[i].size();j++)
		{
			CRtnuUnit *unit=_units[i][j];
			if (!unit)
				continue;

			_UpdateUnit_Knight(unit,_stepOwnerMove,dt);
		}
	}

	float speedMax=speedOwner*1.5f;
	float speedMin=speedOwner*0.8f;

	for (int i=0;i<ARRAY_SIZE(_units);i++)
	{
		for (int j=0;j<_units[i].size();j++)
		{
			CRtnuUnit *unitRtnu=_units[i][j];
			if (unitRtnu)
			{
				LevelPos &dir=unitRtnu->GetMoveDir();
				float dist=unitRtnu->GetMoveDist();

				unitRtnu->_unitRvo->setPrefVelocity(dir*(dist/dt));
				if (dist>0.0f)
					unitRtnu->_unitRvo->setTimeHorizon(0.04f);
				else
					unitRtnu->_unitRvo->setTimeHorizon(0.04f);

 				unitRtnu->_unitRvo->setMaxSpeed((dist/dt));
			}
		}
	}
}

void CLevelRtnuCircum::_UpdateThreats()
{
	if (!_owner)
		return;

	CLoUnit *loOwnerUnit=_owner->GetLoUnit();
	if (!loOwnerUnit)
		return;


	LevelUtilDetectParam param;

	param.loSrc=loOwnerUnit;
	param.pos=loOwnerUnit->GetFramePos();
	param.rangeMin=0.0f;
	param.rangeMax=20.0f;
	LevelDetectTargetFlag flags=((LevelDetectTargetFlag)(LevelDetectTarget_Enemy|LevelDetectTarget_Unit|LevelDetectTarget_Player|LevelDetectTarget_Ground|LevelDetectTarget_Float));
	param.flags=&flags;
	param.nFlags=1;
	LevelObjRequire requires=LevelObjRequire_Attackable;
	param.requires=&requires;
	param.nRequires=1;
	param.weights.Zero();
	param.bTouching=TRUE;

	DWORD c;
	CLevelObj **detects=LevelUtil_Detect(param,NULL,c);

	for (int i=0;i<ARRAY_SIZE(_units);i++)
	{
		for (int j=0;j<_units[i].size();j++)
		{
			CRtnuUnit *unitRtnu=_units[i][j];
			if (!unitRtnu)
				continue;

			CLevelObj *lo=(CLevelObj *)unitRtnu->GetData();
			if (lo)
			{
				CLevelSensor *sensor=lo->GetSensor();

				if (sensor)
				{
					LevelPos &pos=unitRtnu->GetPos();
					float dist2Min=1000000.0f;
					CLevelObj *loBest=NULL;
					for (int k=0;k<c;k++)
					{
						CLevelObj *loDetect=detects[k];
						float dist2=loDetect->GetFramePos().getDistanceSQFrom(pos);
						if (dist2<dist2Min)
						{
							loBest=loDetect;
							dist2Min=dist2;
						}
					}

					sensor->OverrideThreat(loBest,_level->GetT_());
				}
			}
		}
	}


}


void CLevelRtnuCircum::PostSimulate()
{
	if (!_owner)
		return;

	CLoUnit *loOwnerUnit=_owner->GetLoUnit();
	if (!loOwnerUnit)
		return;

	RvoSimulator &sim=_level->GetRvoSim();
	float dt=sim.getTimeStep();

	LevelPos posOwner;
	LevelFace faceOwner;

	posOwner=loOwnerUnit->GetFramePos();
	faceOwner=loOwnerUnit->GetFrameFace();

	for (int i=0;i<ARRAY_SIZE(_units);i++)
	{
		for (int j=0;j<_units[i].size();j++)
		{
			CRtnuUnit *unitRtnu=_units[i][j];
			if (unitRtnu)
			{
				LevelPos &posNew=unitRtnu->_unitRvo->Pos();
				LevelPos dir=posNew-unitRtnu->GetPos();

				BOOL bStuck=FALSE;
				if (posNew.getDistanceSQFrom(unitRtnu->GetPos())<0.005f*0.005f)
					bStuck=TRUE;

				unitRtnu->SetPos(posNew);

				if (unitRtnu->_unitRvo->isStuck()||bStuck)
					unitRtnu->StopMove();

				//更新Face
				if (TRUE)
				{
					CLevelObj *loThreat=NULL;
					CLevelObj *loUnit=(CLevelObj *)unitRtnu->GetData();
					if (loUnit)
					{
						CLevelSensor *sensor=loUnit->GetSensor();
						if (sensor)
							loThreat=sensor->GetThreat();
					}

					if (loThreat&&unitRtnu->AllowSlideMoving())
					{
					}
					else
					{
						if (!bStuck)
						{
							const float spdRot=360.0f*i_math::GRAD_PI2;

							BOOL bNearlyStopping=FALSE;
							if (unitRtnu->_status==CRtnuUnit::Status_Gathering)
							{
								if (unitRtnu->_durGathering<0.1f)
									bNearlyStopping=TRUE;
							}

							if (!bNearlyStopping)
							{
								i_math::rotate_limited(unitRtnu->_faceCur,LevelFaceFromDir(dir),spdRot*dt);
							}
							else
							{
								if (dir.getLength()/dt>unitRtnu->_speed*0.5f)
									i_math::rotate_limited(unitRtnu->_faceCur,LevelFaceFromDir(dir),spdRot*dt);
							}

						}
					}
				}
			}
		}
	}

}
