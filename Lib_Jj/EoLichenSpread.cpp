
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoLichenSpread.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "spline/CubicSpline.h"
#include "Random/Random.h"


BIND_EOPARAM(EoLichenSpread,EoParamLichenSpread);

void EoLichenSpread::_OnPostCreate()
{
	_BuildBranches();

}


void EoLichenSpread::_OnUpdate()
{
	EoParamLichenSpread *param=GetParam<EoParamLichenSpread >();

	LevelTick t=_GetT();

	EoEnv *eo=(EoEnv *)_level->GetEoEnv();


	if (eo)
	{
		for (int i=0;i<_nBranches;i++)
		{
			Branch &branch=_branches[i];

			AnimTick tAge=_GetAge();

			for (int j=0;j<branch.samples.size();j++)
			{
				Sample &sample=branch.samples[j];
				if (sample.h==EoEnvLichenHandle_Invalid)
				{
					if ((tAge>=sample.tStart)&&(tAge<sample.tEnd))
						sample.h=eo->StartLichen(sample.pos,sample.radius,TRUE,1.0f,ANIMTICK_TO_SECOND(param->durFI),ANIMTICK_TO_SECOND(param->durFO));
				}
				else
				{
					if (tAge>=sample.tEnd)
					{
						eo->StopLichen(sample.h);
						sample.h=EoEnvLichenHandle_Invalid;
					}
				}
			}
		}
	}

	if (t>_tCreate+param->dur+param->durStart+param->durEnd+param->durFO+ANIMTICK_FROM_SECOND(5.0f))
		DeferDestroy();
}

void EoLichenSpread::_OnDetroy()
{
	EoEnv *eo=(EoEnv *)_level->GetEoEnv();
	if (eo)
	{
		for (int i=0;i<_nBranches;i++)
		{
			Branch &branch=_branches[i];

			for (int j=0;j<branch.samples.size();j++)
			{
				Sample &sample=branch.samples[j];
				if (sample.h!=EoEnvLichenHandle_Invalid)
					eo->StopLichen(sample.h);
			}
		}
	}
	_nBranches=0;
}

CCubicSpline &EoLichenSpread::GetWorkingSpline()
{
	static CCubicSpline spline;
	return spline;
}


void EoLichenSpread::_BuildBranch(LevelFace faceInitial,Branch &branch)
{
	const float swingMax=30.0f;
	const float gapSample=0.2f;


	EoParamLichenSpread *param=GetParam<EoParamLichenSpread >();

	CUnitMgrNavMesh *unitmgr=_level->GetUnitMgr();
	assert(unitmgr);

	LevelPos posInitial=_GetInitialPos();

	float step=param->lengthBranch/(float)param->nSegsPerBranch;

	CCubicSpline &spline=GetWorkingSpline();
	spline.Reset(FALSE);

	LevelPos pos=posInitial;
	LevelFace face=faceInitial;


	for (int i=0;i<param->nSegsPerBranch;i++)
	{
		BOOL bOk=FALSE;
		for (int k=0;k<5;k++)
		{
			LevelFace faceNext;
			if(i>0)
				faceNext=CSysRandom::RandRange(face-swingMax*i_math::GRAD_PI2,face+swingMax*i_math::GRAD_PI2);
			else
				faceNext=face;
			LevelPos posNext=pos+LevelFaceToDir(faceNext)*step;
			if(!unitmgr->StaticObstacleTest(UnitFindPath_Walkable,pos,posNext))
			{
				LevelPos3D pos3D;
				if(i==0)
					spline.AddNode(posInitial,i_math::quatf());
				spline.AddNode(posNext,i_math::quatf());

				bOk=TRUE;
				pos=posNext;
				face=faceNext;
				break;
			}
		}
		if (!bOk)
			break;
	}

	if(spline.GetNodeCount()<=0)
		return;

	spline.BuildRNS();

	float distTotal=spline.GetDistance();

	static std::vector<CCubicSpline::Sample>samples;
	samples.resize((int)(distTotal/gapSample)+20);

	DWORD nSamples=spline.GetSamples(gapSample,samples.data());

	branch.samples.resize(nSamples);

	for (int i=0;i<nSamples;i++)
	{
		CCubicSpline::Sample &sampleSpline=samples[i];
		Sample &sample=branch.samples[i];

		float dist=gapSample*(float)i;

		sample.h=EoEnvLichenHandle_Invalid;

		sample.pos=sampleSpline.pos.getXZ();
		if (TRUE)
		{
			float r=((float)i)/(float)(nSamples+1);
			r=i_math::clamp_f(1.0f-r,0.0f,1.0f);
			sample.radius=param->radiusMin+r*(param->radiusMax-param->radiusMin);
		}
		
		if (TRUE)
		{
			float r=i_math::clamp_f(dist/param->lengthBranch,0.0f,1.0f);

			sample.tStart=(AnimTick)(r*(float)param->durStart);
			sample.tEnd=(AnimTick)((1.0f-r)*(float)param->durEnd)+param->durStart+param->dur;
		}

	}


}


void EoLichenSpread::_BuildBranches()
{
	EoParamLichenSpread *param=GetParam<EoParamLichenSpread >();

	LevelFace face=LevelFaceFromDir_XZ(GetInitialDir3D());

	LevelFace faceFrom,faceTo;
	float step;
	if (TRUE)
	{
		float half=param->fov*i_math::GRAD_PI2/2.0f;
		step=param->fov*i_math::GRAD_PI2/(float)(param->nBranches-1);
		faceFrom=face-half;
		faceTo=face+half;
	}

	LevelFace faceBranch=faceFrom;
	for (int i=0;i<param->nBranches;i++)
	{
		_BuildBranch(faceBranch,_branches[i]);
		faceBranch+=step;
	}

	_nBranches=param->nBranches;

}
