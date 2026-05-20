/********************************************************************
	created:	2016/6/04 
	author:		cxi
	
	purpose:	监控是否Stuck
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "Level.h"

#include "BgnMonitorStuck.h"

#include "LoUnit.h"
#include "LevelCoSkill.h"

////////////////////////////////////////////////////////////////////////
//CBgn_MonitorStuck
BIND_BGN_CLASS(CBgn_MonitorStuck,CBgp_MonitorStuck);
void CBgn_MonitorStuck::Start(DWORD iStb,BGNOutputs &outputs)
{
	_t=_GetT();
	CLevelObj *lo=_GetLo();
	if (lo)
		_pos=lo->GetFramePos();

}

void CBgn_MonitorStuck::Update(BGNOutputs &outputs)
{
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		CLevelObjMove *move=lo->GetMove();
		if (move)
		{
			BOOL bExpectMoving=FALSE;
			if (TRUE)
			{
				if (move->IsMoving_())
				{
					CLevelBuffs *buffs=lo->GetBuffs();
					if (buffs)
					{
						if (!buffs->TestFlag(BuffFlag_Birth|BuffFlag_Dead|BuffFlag_LayDown|BuffFlag_Pausing|BuffFlag_PausingAnim|BuffFlag_Reside|BuffFlag_Mount))
							bExpectMoving=TRUE;
					}
				}
			}

			if (bExpectMoving)
			{
				AnimTick t=_GetT();
				if (t>_t)
				{
					LevelPos pos=lo->GetFramePos();
					if (pos.getDistanceSQFrom(_pos)<0.01f)
						_nStuck++;
					else
						_nStuck=0;

					_t=t;
					_pos=pos;

					_history.PushBack(_pos);

					if (_nStuck>2)
						_OutputOk(outputs,1,"监控到");
					else
					{
						if (_history.GetCount()>=_history.GetCapacity())
						{
							int c=_history.GetCount();
							i_math::rectf rc;
							for (int i=c-1;i>=0;i--)
							{
								LevelPos pos=_history.GetAt(i);
								rc.merge(pos.x,pos.y);
							}

							float radiusLimit=1.5f;
							float w=rc.getWidth();
							float h=rc.getHeight();

							if (w*w+h*h<radiusLimit*radiusLimit*4.0f)
								_OutputOk(outputs,1,"监控到");
						}
					}
				}
			}
		}
	}

}
