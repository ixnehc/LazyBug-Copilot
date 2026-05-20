/********************************************************************
	created:	2020/02/23 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelBGs.h"

#include "BgnThreat_ForceUpdate.h"

#include "LevelSensor.h"



////////////////////////////////////////////////////////////////////////
//CBgnThreat_ForceUpdate
BIND_BGN_CLASS(CBgnThreat_ForceUpdate,CBgpThreat_ForceUpdate);



void CBgnThreat_ForceUpdate::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelSensor *sensor=_GetLo()->GetSensor();
	if (sensor)
		sensor->ForceUpdate();

	_OutputOk(outputs,1,"结束");
}

