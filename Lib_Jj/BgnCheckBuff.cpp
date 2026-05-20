/********************************************************************
	created:	2013/01/25 
	author:		cxi
	
	purpose:	 检查是否有某个Buff
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelRecordBuff.h"

#include "BgnCheckBuff.h"

//////////////////////////////////////////////////////////////////////////
//CBgp_CheckBuff
void CBgp_CheckBuff::FillDesc(std::string &s,FillDescAssist *assist)
{
	s="n/a";
	if (!_bCheckParamClass)
	{
		if (_nmLo==StringID_Invalid)
			FormatString(s,"检测自己是否有Buff:%s",GetBVRDesc_BuffID(BVR_ARG(_idBuff),assist));
		else
			FormatString(s,"检测游戏对象[%s]中是否有Buff:%s",assist->GetStr(_nmLo),GetBVRDesc_BuffID(BVR_ARG(_idBuff),assist));
	}
	else
	{
		CRecord *rec=assist->GetClonedRec_Buff(_idBuff);
		if (rec)
		{
			LevelRecordBuff *recBuff=(LevelRecordBuff *)rec;
			LevelBuffParam *param=recBuff->param;
			if (_nmLo==StringID_Invalid)
				FormatString(s,"检测自己是否有与%s同类型的(%s)的Buff",GetBVRDesc_BuffID(BVR_ARG(_idBuff),assist),param->GetBuffClass()->GetName());
			else
				FormatString(s,"检测游戏对象[%s]中是否有与%s同类型的(%s)的Buff",assist->GetStr(_nmLo),GetBVRDesc_BuffID(BVR_ARG(_idBuff),assist),param->GetBuffClass()->GetName());

			SAFE_RELEASE(rec);
		}
	}
}


////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckBuff,CBgp_CheckBuff);

void CBgn_CheckBuff::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckBuff*pad=_GetPad<CBgp_CheckBuff>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	if (level)
	{
		if (pad->_nmLo)
		{
			LevelObjID id;
			if (_GetID(pad->_nmLo,BehaviorMemType_ObjID,id))
			{
				if (level)
					lo=_GetLevel()->GetIDs()->LoFromID(id);
			}
		}

		if (lo)
		{
			if (!pad->_bCheckParamClass)
			{
				extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
				if (LevelUtil_FindBuffByRecordID(lo,pad->_idBuff))
				{
					_OutputOk(outputs,1,"有Buff");
					return;
				}
			}
			else
			{
				LevelRecordBuff *recBuff=level->GetRecords()->GetBuff(pad->_idBuff);
				if (recBuff)
				{
					extern CLevelBuff *LevelUtil_FindBuff(CLevelObj *lo,CClass *clssBuff);
					if (LevelUtil_FindBuff(lo,recBuff->param->GetBuffClass()))
					{
						_OutputOk(outputs,1,"有Buff");
						return;
					}
				}
			}
		}
	}

	_OutputFail(outputs,2,"没有Buff");
}

