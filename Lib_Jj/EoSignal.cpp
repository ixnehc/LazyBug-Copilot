
#include "stdh.h"

#include "Level.h"

#include "EoSignal.h"


BIND_EOPARAM(EoSignal,EoParamSignal);

void EoSignal::_OnPostCreate()
{
	EoParamSignal *param=GetParam<EoParamSignal>();
	if (param)
	{
		if (param->nm!=StringID_Invalid)
		{
			CLevelObj *owner=_GetOwner();
			if (owner)
			{
				_level->GetEventMap()->AddSignal(param->nm,owner->GetFramePos(),param->radius,owner->GetID());
			}
		}
	}

	DeferDestroy();
}

