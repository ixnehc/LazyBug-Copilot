/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "LevelBGs.h"

#include "LevelBehavior.h"
#include "BgnStroll.h"
#include "LevelObj.h"

#include "LevelSkillDriver.h"

#include "random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgn_Stroll

BIND_BGN_CLASS(CBgn_Stroll,CBgp_Stroll);
void CBgn_Stroll::Start(DWORD iStb,BGNOutputs &outputs)
{

}

void CBgn_Stroll::Update(BGNOutputs &outputs)
{
	CBgp_Stroll*pad=_GetPad<CBgp_Stroll>();
	AnimTick t=_GetT();
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		CLevelSkillDriver *driver=lo->GetSkillDriver();
		if (driver)
		{
			if (_IsTalkActive())
			{
				driver->StopMove();
				_tNextMove=ANIMTICK_INFINITE;
			}
			else
			{
				if (_tNextMove!=ANIMTICK_INFINITE)
				{
					if (_tNextMove<=t)
					{//选择一个地方移动

						if (TRUE)
						{
							CLevel *lvl=lo->GetLevel();
							LevelPos pos=lo->GetFramePos();

							float rangeX=CSysRandom::RandRange(-pad->range,pad->range);
							float rangeY=CSysRandom::RandRange(-pad->range,pad->range);

							pos.x+=rangeX;
							pos.y+=rangeY;

							LevelSkillTarget target;
							target.SetPos(pos);
							driver->StartFollow(target);
						}

						AnimTick dur=CSysRandom::RandVaryUInt(pad->gap,pad->gapVary);
						_tNextMove=t+dur;
					}
				}
				else
				{
					AnimTick dur=CSysRandom::RandVaryUInt(pad->gap,pad->gapVary);
					_tNextMove=t+dur;
				}
			}
		}
	}

}
