/********************************************************************
	created:	2018/8/29 
	author:		cxi
	
	purpose:	GA战斗环境
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelUtil.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"


#include "LevelOSB.h"

#include "LoEffectObj.h"

#include "BgnGA_Env.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "EoEnv.h" 


////////////////////////////////////////////////////////////////////////
//CBgnGA_Env
BIND_BGN_CLASS(CBgnGA_Env,CBgpGA_Env);

void CBgnGA_Env::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_Env*pad=_GetPad<CBgpGA_Env>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	if (level&&lo)
	{
		if (pad->_op==CBgpGA_Env::Op_Create)
		{
			LevelObjID idEnv=LevelUtil_CreateEnvEo(lo,pad->_idEo);
			if (idEnv)
			{
				CLevelObj *lo=LevelUtil_GetAliveLo(level,idEnv);
				if (lo)
				{
					if (lo->GetClass()->IsSameWith(Class_Ptr2(EoEnv)))
					{
						((EoEnv*)lo)->SetArea(pad->_area);
					}
				}
			}
		}
		if (pad->_op==CBgpGA_Env::Op_Destroy)
			LevelUtil_DestroyEnvEo(level);
	}

	_OutputOk(outputs,1,"结束");
	return;

}

//////////////////////////////////////////////////////////////////////////
//CBgnGA_Env_SetFenceDestroyed
BIND_BGN_CLASS(CBgnGA_Env_SetFenceDestroyed,CBgpGA_Env_SetFenceDestroyed);
void CBgnGA_Env_SetFenceDestroyed::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_Env_SetFenceDestroyed*pad=_GetPad<CBgpGA_Env_SetFenceDestroyed>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	if (level&&lo)
	{
		CLevelObj *lo=level->GetEoEnv();
		if (lo)
		{
			if (lo->GetClass()->IsSameWith(Class_Ptr2(EoEnv)))
				((EoEnv*)lo)->SetFenceDestroyed();
		}
	}

	_OutputOk(outputs,1,"结束");
	return;

}


//////////////////////////////////////////////////////////////////////////
//CBgnGA_Env_Check
BIND_BGN_CLASS(CBgnGA_Env_Check,CBgpGA_Env_Check);
void CBgnGA_Env_Check::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_Env_Check*pad=_GetPad<CBgpGA_Env_Check>();

	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	if (level&&lo)
	{
		CLevelObj *loEnv=level->GetEoEnv();
		if (pad->op==0)
		{
			if (loEnv)
			{
				if (loEnv->GetClass()->IsSameWith(Class_Ptr2(EoEnv)))
				{
					if (((EoEnv*)loEnv)->CheckFenceDestroyed())
					{
						_OutputOk(outputs,1,"是");
						return;
					}
				}
			}
		}
		if (pad->op==1)
		{
			if (loEnv&&loEnv->IsAlive())
			{
				_OutputOk(outputs,1,"是");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"否");
	return;

}
