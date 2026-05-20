
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoWave.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoWave,EoParamWave);


void EoWave::_OnUpdate()
{
	EoParamWave *param=GetParam<EoParamWave>();
	if (!param)
		return;

	if (_radiusCur>=param->radius)
	{
		if (ANIMTICK_TO_SECOND(_level->GetT_()-_tCreate)>param->radius/param->speed+1.0f)
			DeferDestroy();
		return;
	}

	AnimTick tAge=_level->GetT_()-_tCreate;
	float radius=ANIMTICK_TO_SECOND(tAge)*param->speed;
	if (radius>param->radius)
		radius=radius;

	float radiusCur2=_radiusCur*_radiusCur;
	float radius2=radius*radius;

	if (TRUE)
	{
		DWORD c;
		CLevelObj **los=_DetectRange(GetFramePos(),param->radius,c);

		LevelPos dirInitial=_GetInitialDir();
		LevelFace faceInitial=LevelFaceFromDir(dirInitial);

		float rangeFace=param->fov*i_math::GRAD_PI2/2.0f;

		for (int i=0;i<c;i++)
		{
			CLevelObj *loTarget=los[i];

			if (_damaged.find(loTarget->GetID())!=_damaged.end())
				continue;//已经伤害过了

			LevelPos dir=loTarget->GetFramePos()-_GetInitialPos();
			float dist2=dir.getLengthSQ();

// 			if (dist2<radiusCur2)
// 				continue;
			if (dist2>=radius2)
				continue;

			LevelFace face=LevelFaceFromDir(dir);

			float gap=i_math::get_radian_dist(face,faceInitial);
			if (gap>rangeFace)
				continue;

			_damaged.insert(loTarget->GetID());

			DealArg arg;
			arg.dir.setXZ(dir.safe_normalize());
			arg.link.id=GetLevel()->GenOpLinkID();
			arg.link.t=tAge;
			arg.grd=0;

			_MakeDeals(loTarget,arg);
		}
	}

	_radiusCur=radius;


}

