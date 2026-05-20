
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"
#include "LevelTroops.h"

#include "EoRavens.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "Random/Random.h"
#include "timer/timer.h"



//////////////////////////////////////////////////////////////////////////
//EoRavens
BIND_EOPARAM(EoRavens,EoParamRavens);


void EoRavens::_OnPostCreate()
{
	EoParamRavens *param=GetParam<EoParamRavens>();

	CLevelObj *owner=_GetOwner();
	_hpOwner=(int)LevelUtil_GetCurHP(owner);
	if (_hpOwner<=0)
		_hpOwner=1;

	if (owner)
	{
		CLevelTroop *troop=owner->GetTroop();
		if (troop)
			troop->AddUnit(LevelTroopRank_Minion,GetID());
	}

	_Build();

}

void EoRavens::OnDestroy()
{
	_pathes.clear();
}

void DetectFurthestDirection(CUnitMgrNavMesh *unitmgr,LevelPos &posSrc,LevelFace from,LevelFace to,float distMax,LevelFace &faceResult,float &distResult)
{
	distResult=0.0f;

	const float distReserve=1.0f;
	float distRay=distMax+distReserve;

	float step=30.0f*i_math::GRAD_PI2;

	int nSeg=(int)((to-from)/step)+1;
	step=(to-from)/(float)nSeg;
	BOOL bLoop=FALSE;
	if (i_math::equals(from,to-i_math::Pi*2.0f,0.001f))
		bLoop=TRUE;

	int nStep=nSeg;
	if (!bLoop)
		nStep++;

	int index=CSysRandom::RandRangeInt(0,nStep);
	extern int GenPrimeStep();
	int deltaIndex=GenPrimeStep();

	for (int i=0;i<nStep;i++)
	{
		LevelFace face=from+step*(float)index;
		face=CSysRandom::RandVary(face,step*0.5f);
		index=(index+deltaIndex)%nStep;
		LevelPos posTarget=posSrc+LevelFaceToDir(face)*distRay;

		LevelPos posHit;
		if (!unitmgr->StaticRayCast(UnitFindPath_Walkable,posSrc,posTarget,posHit))
		{
			faceResult=face;
			distResult=distMax;
			return;
		}

		float dist=posHit.getDistanceFrom(posSrc)-distReserve;
		if (dist<0.001f)
			dist=0.001f;
		if (dist>distResult)
		{
			distResult=dist;
			faceResult=face;
		}
	}
}

void EoRavens::_Build(RavenPath &path)
{
	EoParamRavens *param=GetParam<EoParamRavens>();

	CLevelObj *owner=_GetOwner();

	LevelPos3D posInitial3D=_GetInitialPos3D();
	LevelPos posInitial=posInitial3D.getXZ();
	LevelPos dirInitial=_GetInitialDir();

	CUnitMgrNavMesh *unitmgr=_level->GetUnitMgr();

	CPathSpline splineWorking;
	BOOL bOk=FALSE;

	LevelPos posTarget;
	BOOL bGuide=FALSE;
	LevelPos posGuide;
	if (TRUE)
	{
		const float distMax=CSysRandom::RandRange(param->radiusMin,param->radiusMax);

		LevelPos posCur=posInitial;
		float faceCur=0.0f;

		float distAccum=0.0f;

		if (param->distGuide<=0.0f)
		{
			for (int i=0;i<6;i++)
			{
				LevelFace faceFurthest;
				float distFurthest;
				if (i==0)
					DetectFurthestDirection(unitmgr,posCur,0.0f,i_math::Pi*2.0f,distMax,faceFurthest,distFurthest);
				else
					DetectFurthestDirection(unitmgr,posCur,faceCur-i_math::deg2rad(75.0f),faceCur+i_math::deg2rad(75.0f),distMax-distAccum,faceFurthest,distFurthest);
				posCur=posCur+LevelFaceToDir(faceFurthest)*distFurthest;
				faceCur=faceFurthest;
				distAccum+=distFurthest;

				if (distAccum>=distMax)
					break;
			}
			posTarget=posCur;
		}
		else
		{
			float faceInitial=LevelFaceFromDir(dirInitial);
			LevelFace faceFurthest;
			float distFurthest;
			DetectFurthestDirection(unitmgr,posCur,faceInitial-i_math::deg2rad(75.0f),faceInitial+i_math::deg2rad(75.0f),distMax-distAccum,faceFurthest,distFurthest);
			posCur=posCur+LevelFaceToDir(faceFurthest)*distFurthest;
			if (distFurthest>0.2f)
			{
				bGuide=TRUE;
				posGuide=posCur;
			}

			faceCur=faceFurthest;

			for (int i=0;i<6;i++)
			{
				DetectFurthestDirection(unitmgr,posCur,faceCur-i_math::deg2rad(75.0f),faceCur+i_math::deg2rad(75.0f),distMax-distAccum,faceFurthest,distFurthest);
				posCur=posCur+LevelFaceToDir(faceFurthest)*distFurthest;
				faceCur=faceFurthest;
				distAccum+=distFurthest;
				if (distAccum>=distMax)
					break;
			}
			posTarget=posCur;
		}
	}

	if (TRUE)
	{
		splineWorking.Reset();
		splineWorking.AddInitialPos(posInitial);
		if (bGuide)
			splineWorking.AddInitialPos(posGuide);
		splineWorking.Build(unitmgr,posTarget);

		//加一点弧线
		LevelPos3D posMiddle3D;
		if (TRUE)
		{
			float dist=splineWorking.GetDistance();
			float distGuide=0.0f;
			if (bGuide)
				distGuide=posGuide.getDistanceFrom(posInitial);
			LevelPos3D dirMiddle3D;
			splineWorking.Sample(distGuide+(dist-distGuide)*0.5f,posMiddle3D,dirMiddle3D);
			LevelPos dirMiddle=dirMiddle3D.getXZ();
			dirMiddle.safe_normalize();

			if (CSysRandom::Roll(0.5f))
				dirMiddle.rotateBy(90.0f,i_math::vector2df(0.0f,0.0f));
			else
				dirMiddle.rotateBy(-90.0f,i_math::vector2df(0.0f,0.0f));

			LevelPos posMiddle=posMiddle3D.getXZ();
			unitmgr->MoveTo(UnitFindPath_Walkable,posMiddle,dirMiddle*5.0f);
			posMiddle3D.setXZ(posMiddle);
		}

		splineWorking.Reset();
		splineWorking.AddInitialPos(posInitial);
		if (bGuide)
			splineWorking.AddInitialPos(posGuide);
		splineWorking.Build(unitmgr,posMiddle3D);

		splineWorking.Confirm(splineWorking.GetDistance()*0.75f);
		splineWorking.Build(unitmgr,posTarget);
	}

	path.posTarget=posTarget;

	float htAim=owner->GetAimHeight();
	if (TRUE)
	{
		float distSpline=splineWorking.GetDistance();
		if (param->vsVer.GetKeyCount()>0)
		{
			for (int i=0;i<param->vsVer.GetKeyCount();i++)
			{
				Key_f *k=(Key_f *)param->vsVer.GetKey(i);
				float t=ANIMTICK_TO_SECOND(k->t);
				float v=k->v;

				LevelPos pos;
				splineWorking.Sample(t*distSpline,pos);

				LevelPos3D pos3D=LevelUtil_GetWalkableGroundHeight(_level,pos.x,pos.y,TRUE);

				float ht=v*distSpline;
				pos3D.y+=ht;
				pos3D.y+=htAim;

				path.spline.AddNode(pos3D,i_math::quatf());
			}
		}
		else
		{
			path.spline.AddNode(posInitial,i_math::quatf());
			path.spline.AddNode(posTarget,i_math::quatf());
		}
	}

	path.spline.BuildRNS();
	path.bDealt=FALSE;

}


void EoRavens::_Build()
{
	EoParamRavens *param=GetParam<EoParamRavens>();

	DWORD c=CSysRandom::RandRangeInt(param->countMin,param->countMax+1);
	_pathes.resize(c);

	for (int i=0;i<c;i++)
		_Build(_pathes[i]);
	_idxGenuine=CSysRandom::RandRangeInt<int>(0,_pathes.size());
}



LevelPos EoRavens::GetFramePos()
{
	EoParamRavens *param=GetParam<EoParamRavens>();

	return _GetInitialPos();

}

void EoRavens::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bContent=TRUE;

	bp->Data_NextByte()=_pathes.size();
	for (int i=0;i<_pathes.size();i++)
	{
		RavenPath &path=_pathes[i];
		WORD c=(WORD)path.spline.GetNodeCount();
		bp->Data_NextWord()=c;
		for (int j=0;j<c;j++)
			bp->Data_WriteSimpleR(path.spline.GetNode(j)->position);
	}
}


void EoRavens::_OnUpdate()
{
	EoParamRavens *param=GetParam<EoParamRavens>();

	if (TRUE)
	{
		BOOL bEnv=FALSE;
		if (_level)
		{
			CLevelObj *loEnv=_level->GetEoEnv();
			if (loEnv&&loEnv->IsAlive())
				bEnv=TRUE;
		}
		if(!bEnv)
		{
			DeferDestroy();
			return;
		}
	}


	AnimTick tAge=_GetAge();

	for (int i=0;i<_pathes.size();i++)
	{
		RavenPath &path=_pathes[i];
		if (path.bDealt)
			continue;
		float dist=path.spline.GetDistance();

		AnimTick dur=ANIMTICK_FROM_SECOND(dist/param->speed);
		if (tAge>=dur)
		{
			DealArg arg;
			arg.link.id=GetLevel()->GenOpLinkID();
			arg.link.iSerial=i;
			arg.hpInitial=_hpOwner;
			
			if (_idxGenuine==i)
				MakeDeals(param->summon,LevelOSB(this),LevelUtil_GetWalkableGroundHeight(_level,path.posTarget.x,path.posTarget.y,TRUE),arg,NULL);
			else
				MakeDeals(param->summonImposter,LevelOSB(this),LevelUtil_GetWalkableGroundHeight(_level,path.posTarget.x,path.posTarget.y,TRUE),arg,NULL);

			path.bDealt=TRUE;
		}
	}

	BOOL bCanDestroy=TRUE;
	for (int i=0;i<_pathes.size();i++)
	{
		RavenPath &path=_pathes[i];
		if (!path.bDealt)
		{
			bCanDestroy=FALSE;
			break;
		}

		float dist=path.spline.GetDistance();
		AnimTick dur=ANIMTICK_FROM_SECOND(dist/param->speed);
		if (tAge<dur+ANIMTICK_FROM_SECOND(4.0f))
		{
			bCanDestroy=FALSE;
			break;
		}
	}

	if (bCanDestroy)
		DeferDestroy();

}
