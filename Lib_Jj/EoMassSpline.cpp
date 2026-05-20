
#include "stdh.h"

#include "math/circle.h"
#include "Log/LogDump.h"

#include "Level.h"
#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoMassSpline.h"

#include "EoSplineBullet.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "Random/Random.h"



//////////////////////////////////////////////////////////////////////////
//EoMassSpline

BIND_EOPARAM(EoMassSpline,EoParamMassSpline);

void EoMassSpline::_OnPostCreate()
{
	CLevelSkill *skill=_GetOwnerSkill();
	CUnitMgrNavMesh *unitmgr=_level->GetUnitMgr();
	assert(unitmgr);

	BOOL bOk=FALSE;
	EoParamMassSpline *param=_rec->GetParam<EoParamMassSpline>();
	if (param)
	{
		if (skill)
		{
			AnimEventZone::KeyFan kFan;
			i_math::vector3df dir;
			if (_CalcEZoneInfo(kFan,_posInitial,dir,_fov))
			{
				_faceInitial=LevelFaceFromDir_XZ(dir);
				LevelSkillTarget &target=skill->GetTarget();
				LevelPos posTarget;
				if (LevelUtil_CalcTargetPos(_level,target,posTarget))
				{
					_spline.AddInitialPos(_posInitial);
					_spline.Build(unitmgr,posTarget);

					_dur=_eZone->GetDur();
					_tStart=_GetSkillCastingTime();
					bOk=TRUE;
				}
			}
		}
	}

	if (!bOk)
	{
		DeferDestroy();
		return;
	}

}


void EoMassSpline::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Data_NextDword()=_idSrcOwner;
}

void EoMassSpline::_OnUpdate()
{
	EoParamMassSpline *param=_rec->GetParam<EoParamMassSpline>();

	if (param->idEo==RecordID_Invalid)
		return;

	AnimTick tCasting=_GetSkillCastingTime();
	AnimTick tAge=ANIMTICK_SAFE_MINUS(tCasting,_tStart);

	if (tAge>_dur)
	{
		DeferDestroy();
		return;
	}

	CUnitMgrNavMesh *unitmgr=_level->GetUnitMgr();
	assert(unitmgr);

	float distMax=_spline.GetDistance();
	float distMin=3.0f;
	if (distMin>distMax)
		distMin=distMax;

	//Update spawn route
	if (TRUE)
	{
		int nToSpawn=1+(int)(ANIMTICK_TO_SECOND(tAge)*param->spdSpawn);
		for (int i=_nSpawned;i<nToSpawn;i++)
		{
			float tSpawn=((float)i)*(1.0f/param->spdSpawn);

			float distOnSpline=i_math::lerp(distMin,distMax,tSpawn/ANIMTICK_TO_SECOND(_dur));

			LevelPos posCenter;
			_spline.Sample(distOnSpline,posCenter);

			BOOL bFound=FALSE;
			LevelPos posTarget;
			if (TRUE)
			{
				float radius;
				radius=tan(_fov*0.5f)*distOnSpline;

				float distClosestMax=0.0f;
				DWORD nTry=5;
				for (int j=0;j<nTry;j++)
				{
					LevelPos posCandidate;
					if (TRUE)
					{
						LevelFace face=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
						LevelPos posNearby=posCenter+LevelFaceToDir(face)*radius;
						LevelPos posHit;
						if (unitmgr->StaticRayCast(UnitFindPath_Walkable,posCenter,posNearby,posHit))
							posNearby=posHit;

						posCandidate=posNearby.getInterpolated(posCenter,CSysRandom::RandRange(0.0f,1.0f));
					}

					float distClosest=100000.0f;
					for (int k=0;k<_history.size();k++)
					{
						float dist=_history[k].posTarget.getDistanceFrom(posCandidate);
						if (dist<distClosest)
							distClosest=dist;
					}

					if (distClosest>distClosestMax)
					{
						bFound=TRUE;
						posTarget=posCandidate;
						distClosestMax=distClosest;
					}
				}
			}

			if (!bFound)
			{
				posTarget=posCenter;
				bFound=TRUE;
			}

			if (TRUE)
			{
				History e;
				e.posTarget=posTarget;
				_history.push_back(e);
			}

			LevelPos posGuide;
			if (TRUE)
			{
//				float distGuide=CSysRandom::RandRange(1.0f,4.0f);
				float distGuide=i_math::lerp(1.0f,4.0f,tSpawn/ANIMTICK_TO_SECOND(_dur));
				if (distGuide>distOnSpline*0.5f)
					distGuide=distOnSpline*0.5f;

				_spline.Sample(distGuide,posGuide);
			}

			//Do the spawn
			if (bFound)
			{
				LevelRecordEo *rec=_level->GetRecords()->GetEo(param->idEo);
				if (rec)
				{
					if (rec->param->GetEoClass()->IsSameWith(Class_Ptr2(EoSplineBullet)))
					{
						EoSplineBullet *eo=(EoSplineBullet*)_level->CreateObj(rec->param->GetEoClass());
						if (eo)
						{
							LevelOpLink link;
							link.id=_level->GenOpLinkID();
							link.t=ANIMTICK_FROM_SECOND(tSpawn);
							LevelPos3D dir;
							dir.setXZ(LevelFaceToDir(_faceInitial));
							eo->PostCreate(GetPlayerID(),param->idEo,_posInitial,dir,1,LevelOSB(this),link);
							eo->BuildSpline(_posInitial.getXZ(),posGuide,posTarget);
							_level->AddToActives(eo);

							History e;
							e.posTarget=posTarget;
							_history.push_back(e);
						}
					}
				}
			}
		}

		_nSpawned=nToSpawn;

	}




}
