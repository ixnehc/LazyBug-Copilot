
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoLichen.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoLichen,EoParamLichen);

void EoLichen::_OnPostCreate()
{
	EoParamLichen *param=GetParam<EoParamLichen >();
	EoEnv *eo=(EoEnv *)_level->GetEoEnv();
	if (eo)
		_hLichen=eo->StartLichen(_GetInitialPos(),param->radius,TRUE,1.0f,ANIMTICK_TO_SECOND(param->durFI),ANIMTICK_TO_SECOND(param->durFO));

}


void EoLichen::_OnUpdate()
{
	EoParamLichen *param=GetParam<EoParamLichen >();

	if (_GetT()>_tCreate+param->dur+param->durFI)
		DeferDestroy();
}

void EoLichen::_OnDetroy()
{
	if (_hLichen!=EoEnvLichenHandle_Invalid)
	{
		EoEnv *eo=(EoEnv *)_level->GetEoEnv();
		if (eo)
			eo->StopLichen(_hLichen);
		_hLichen=EoEnvLichenHandle_Invalid;
	}
}
