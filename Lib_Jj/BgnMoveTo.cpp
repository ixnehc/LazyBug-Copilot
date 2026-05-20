/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelBehavior.h"
#include "LevelObj.h"
#include "Level.h"
#include "BgnMoveTo.h"

#include "LevelSkillDriver.h"
#include "LoUnit.h"


////////////////////////////////////////////////////////////////////////
//CBgnMoveTo
BIND_BGN_CLASS(CBgnMoveTo,CBgpMoveTo);


void CBgnMoveTo::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpMoveTo*pad=_GetPad<CBgpMoveTo>();
	CLevel *level=_GetLevel();

	LevelPos posTarget;
	if(_GetPos(pad->_varPos,posTarget))
	{
		CLevelObj *lo=_GetLo();
		if (lo)
		{
			extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
			if (!LevelUtil_CheckDead(lo))
			{
				if (lo->GetType()==LevelObjType_Unit)
				{
					CLoUnit *loUnit=(CLoUnit*)lo;
					CLevelSkillDriver *driver=loUnit->GetSkillDriver();
					if (driver)
					{
						LevelSkillTarget target;
						target.SetPos(posTarget);
						_verCast=driver->GetCastVer();
						if (driver->StartFollow(target,pad->_radiusReach))
						{
							if (!pad->_bMonitorReach)
							{
								_OutputOk(outputs,1,"成功");
								return;
							}
							_radiusReach=pad->_radiusReach;
							_posTarget=posTarget;
							return;
						}
					}
				}
			}
		}
	}

	_OutputFail(outputs,2,"失败");
	
}

void CBgnMoveTo::Update(BGNOutputs &outputs)
{
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
		if (!LevelUtil_CheckDead(lo))
		{
			CLevelSkillDriver *driver=lo->GetSkillDriver();
			if (driver)
			{
				if (driver->GetCastVer()!=_verCast)
				{
					if (lo->GetFramePos().getDistanceSQFrom(_posTarget)<=_radiusReach*_radiusReach)
					{
						_OutputOk(outputs,1,"成功");
						return;
					}
					else
					{
						_OutputFail(outputs,2,"失败");
					}
				}
				return;
			}
		}
	}

	_OutputFail(outputs,2,"失败");
}
