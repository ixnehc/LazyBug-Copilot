
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoAirBomb.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoAirBomb,EoParamAirBomb);

void EoAirBomb::_OnUpdate()
{
	EoParamAirBomb *param=GetParam<EoParamAirBomb>();
	if (!param)
		return;

	if (!_bBurst)
	{
		if (_level->GetT_()>=_tCreate)
		{
			_bBurst=TRUE;

			_MakeRangeDeal3D(param->radius);
		}
	}
	else
	{
		if (_level->GetT_()>_tCreate+ANIMTICK_FROM_SECOND(4.0f))
			DeferDestroy();
	}

}

