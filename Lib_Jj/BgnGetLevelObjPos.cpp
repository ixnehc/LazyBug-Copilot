/********************************************************************
	created:	2013/01/25 
	author:		cxi
	
	purpose:	 检查HP的范围
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LoUnit.h"

#include "BgnGetLevelObjPos.h"

////////////////////////////////////////////////////////////////////////
//CBgn_GetLevelObjPos

BIND_BGN_CLASS(CBgn_GetLevelObjPos,CBgp_GetLevelObjPos);

void CBgn_GetLevelObjPos::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_GetLevelObjPos*pad=_GetPad<CBgp_GetLevelObjPos>();

	if (pad->nmVar!=StringID_Invalid)
	{
		if (pad->nmLo)
		{
			LevelObjID id;
			if (_GetID(pad->nmLo,BehaviorMemType_ObjID,id))
			{
				if (_GetLevel())
				{
					CLevelObj *lo=_GetLevel()->GetIDs()->LoFromID(id);
					if (lo)
					{
						if (lo->IsAlive())
						{
							LevelPos pos=lo->GetFramePos();
							if (_SetPos(pad->nmVar,pos))
							{
								_OutputOk(outputs,1,"成功");
								return;
							}
						}
					}
				}
			}
		}
		else
		{
			LevelPos pos=_GetLo()->GetFramePos();
			if (_SetPos(pad->nmVar,pos))
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"失败");
}


