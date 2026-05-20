
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoBulletBomb.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "Random/Random.h"


BIND_EOPARAM(EoBulletBomb,EoParamBulletBomb);

void EoBulletBomb::_OnUpdate()
{
	EoParamBulletBomb *param=GetParam<EoParamBulletBomb>();
	if (!param)
		return;

	if (!_bBurst)
	{
		if (_level->GetT_()>=_tCreate+param->delay)
		{
			_bBurst=TRUE;

			_MakeRangeDeal(param->radius);
		}
	}

	if (_bBurst)
	{
		int nToBurst=0;
		if (param->dur==0)
			nToBurst=param->nBullet;
		else
		{
			AnimTick tBurst=ANIMTICK_SAFE_MINUS(_level->GetT_(),_tCreate+param->delay);
			nToBurst=(tBurst+1)*param->nBullet/param->dur;
		}

		LevelPos3D pos=_GetInitialPos3D();
		for (int i=_nBurst;i<nToBurst;i++)
		{
			LevelFace face=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
			DealArg arg;
			arg.dir.setXZ(LevelFaceToDir(face));
			_MakeDeals(pos,arg);
		}
		_nBurst=nToBurst;
	}

	if (_nBurst>=param->nBullet)
	{
		if (_level->GetT_()>_tCreate+param->delay+param->dur+ANIMTICK_FROM_SECOND(1.0f))
			DeferDestroy();
	}

}

