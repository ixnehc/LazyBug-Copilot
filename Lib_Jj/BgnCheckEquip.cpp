/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelRecordItemClass.h"

#include "BgnCheckEquip.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckEquip,CBgp_CheckEquip);

void CBgn_CheckEquip::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckEquip*pad=_GetPad<CBgp_CheckEquip>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		ExprEquips *equips=lo->GetExprEquips();
		if (equips)
		{
			LevelRecordItemClass *rec=level->GetRecords()->GetItemClassOfItem(pad->_idItem);
			if (rec)
			{
				if (rec->part<EquipPart_MaxExpress)
				{
					if (equips->items[rec->part]==pad->_idItem)
					{
						_OutputOk(outputs,1,"已装备");
						return;
					}
				}
			}
		}
	}

	_OutputOk(outputs,2,"未装备");
}

