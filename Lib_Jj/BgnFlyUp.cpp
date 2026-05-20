/********************************************************************
	created:	2012/12/17 
	author:		cxi
	
	purpose:	Fly Up
*********************************************************************/
#include "stdh.h"
#include "LevelBGs.h"

#include "BgnFlyUp.h"


#include "Level.h"


BIND_BGN_CLASS(CBgn_FlyUp,CBgp_FlyUp);



void CBgn_FlyUp::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_FlyUp*pad=_GetPad<CBgp_FlyUp>();

	_tStart=_GetT();

	CLevelSkillDriver *driver=_GetSkillDriver();
	if (driver)
	{
		LevelSkillTarget target;
		driver->Start(LevelSkillType(pad->_idSkill),target,FALSE,ClientSkillID_Invalid,LevelSkillGrade_Invalid,NULL);
	}
	else
	{
		_OutputFail(outputs,2,"失败");
	}
}

void CBgn_FlyUp::Update(BGNOutputs &outputs)
{
	CBgp_FlyUp*pad=_GetPad<CBgp_FlyUp>();
	CLevelObj *lo=_GetLo();
	AnimTick t=_GetT();

	if (lo)
	{
		LevelMoveMethod method=lo->GetMoveMethod();

		if (method!=LevelMoveMethod_Flying)
		{
			if (t-_tStart>ANIMTICK_FROM_SECOND(2.0f))
			{
				_OutputFail(outputs,2,"失败");
			}
			return;
		}

		if (_tStartFly==ANIMTICK_INFINITE)
			_tStartFly=t;

		if (t-_tStartFly>pad->_dur)
		{
			_OutputOk(outputs,1,"成功");
			return;
		}
	}
}
