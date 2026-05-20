/********************************************************************
	created:	2018/8/29 
	author:		cxi
	
	purpose:	Lichen控制
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"


#include "LevelOSB.h"

#include "LoEffectObj.h"

#include "EoEnv.h"

#include "BgnLichen.h"

#include "LevelObj.h"
#include "LevelBGs.h"


////////////////////////////////////////////////////////////////////////
//CBgnLichen
BIND_BGN_CLASS(CBgnLichen,CBgpLichen);

void CBgnLichen::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpLichen*pad=_GetPad<CBgpLichen>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	if (level&&lo)
	{
		if (pad->_op==CBgpLichen::Op_CreateBound)
		{
			EoEnv *eo=(EoEnv *)level->GetEoEnv();
			if (eo)
			{
				EoEnvLichenHandle h=eo->StartLichen(lo->GetID(),pad->_radius,FALSE,0.0f);
				_SetID(pad->_nmVar,BehaviorMemType_ObjID,h);
			}
		}
		if (pad->_op==CBgpLichen::Op_CreateTrail)
		{
			EoEnv *eo=(EoEnv *)level->GetEoEnv();
			if (eo)
			{
				EoEnvLichenHandle h=eo->StartLichenTrail(lo->GetID(),pad->_radius);
				_SetID(pad->_nmVar,BehaviorMemType_ObjID,h);
			}
		}
		if (pad->_op==CBgpLichen::Op_Destroy)
		{
			EoEnv *eo=(EoEnv *)level->GetEoEnv();
			if (eo)
			{
				EoEnvLichenHandle h;
				if (_GetID(pad->_nmVar,BehaviorMemType_ObjID,h))
				{
					eo->StopLichen(h);
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;

}
