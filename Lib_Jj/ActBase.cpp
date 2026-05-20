/********************************************************************
	created:	2012/10/24 
	author:		cxi
	
	purpose:	Act Base
*********************************************************************/
#include "stdh.h"

#include "ActBase.h"
#include "LevelRecordAI.h"

#include "LevelSkillDriver.h"


void ActBase::Set(CLevelObj *owner,ActParam *param)
{
	_owner=owner;
	if (param)
	{
		_param=(ActParam *)(param->GetClass()->New());
		_param->CopyFrom(param);
	}
	if (_param)
		assert(GetParamClass()->IsSameWith(param->GetClass()));
}

