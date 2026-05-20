
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoToeStoneThrusts.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "Random/Random.h"


BIND_EOPARAM(EoToeStoneThrusts,EoParamToeStoneThrusts);

void EoToeStoneThrusts::SetSites(i_math::pos2d_sh &ptBase, i_math::pos2db *sites, DWORD nSites)
{
    if (nSites > MAX_TOESTONE_THRUST_SITES)
        nSites = MAX_TOESTONE_THRUST_SITES;
    for (int i = 0;i < nSites;i++)
    {
        int x = ptBase.x + sites[i].x;
        int y = ptBase.y + sites[i].y;

        _sites[i].x = 0.25f + 0.5f*(float)x;
        _sites[i].y = 0.25f + 0.5f*(float)y;
    }
    _nSites = nSites;

    CSysRandom::GenRandomIndices<char>(_indices, _nSites);
}


void EoToeStoneThrusts::_OnUpdate()
{
	EoParamToeStoneThrusts *param=GetParam<EoParamToeStoneThrusts>();
	if (!param)
		return;

    CLevelDecider *decider = _level->GetDecider();

    AnimTick tAge = ANIMTICK_SAFE_MINUS(_level->GetT_(), _tCreate);
    int nToCommit=(int)(ANIMTICK_TO_SECOND(tAge)*param->nThrustsPerSec);
	if (nToCommit>_nSites)
		nToCommit=_nSites;

    while(nToCommit>_nCommits)
	{
        i_math::vector2df &pos = _sites[_indices[_nCommits]];

		DealArg arg;
		arg.link.id=GetLevel()->GenOpLinkID();
        arg.link.iSerial = (BYTE)_nCommits;

		extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
        LevelPos3D pos3D = LevelUtil_GetGroundHeight(_level, pos.x, pos.y, TRUE);
		arg.dir.set(0, 1, 0);
        _MakeDeals(pos3D, arg);
        param->dealThrust->Make(LevelOSB(this), pos3D, arg,NULL);

		_nCommits++;

	}

    float durTotal = ((float)_nSites) / param->nThrustsPerSec;
	if (_level->GetT_()>_tCreate+ANIMTICK_FROM_SECOND(durTotal)+ANIMTICK_FROM_SECOND(1.0f))
	{
		DeferDestroy();
	}
}

