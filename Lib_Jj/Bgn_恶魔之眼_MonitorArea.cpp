/********************************************************************
	created:	2022/04/02 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordAgent.h"

#include "LevelUtil.h"


#include "Bgn_恶魔之眼_MonitorArea.h"
#include "Bgn_恶魔之眼_Update.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "Random/Random.h"



////////////////////////////////////////////////////////////////////////
//CBgn_恶魔之眼_Update
BIND_BGN_CLASS(CBgn_DevilEye_MonitorArea,CBgp_DevilEye_MonitorArea);

BMO_DevilEyeStatus *CBgn_DevilEye_MonitorArea::_GetDevilEyeStatus(CLoGeneralAgent *loAgent)
{
	CBehaviorMem *mem=loAgent->GetBehaviorMem(0);
	if (!mem)
		return NULL;

	return mem->GetObj<BMO_DevilEyeStatus>(_varStatus);
}

BOOL CBgn_DevilEye_MonitorArea::_CheckDevilEye(CLevelObj *lo)
{
	if (lo->GetType()!=LevelObjType_Agent)
		return FALSE;

	if (!IsClass2(lo,CLoGeneralAgent))
		return FALSE;

	CLoGeneralAgent*loAgent=(CLoGeneralAgent*)lo;

	CBgp_DevilEye_MonitorArea*pad=_GetPad<CBgp_DevilEye_MonitorArea>();
	if (!loAgent->GetRec())
		return FALSE;
	if(loAgent->GetRec()->GetID()!=pad->idDevilEye)
		return FALSE;

	return TRUE;
}


void CBgn_DevilEye_MonitorArea::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_DevilEye_MonitorArea*pad=_GetPad<CBgp_DevilEye_MonitorArea>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	_varStatus=pad->varStatus;

	if (level->GetIDs())
	{
		LevelIDsEnumCallBack dlgt;
		dlgt.bind(this,&CBgn_DevilEye_MonitorArea::_CheckDevilEye);

		level->GetIDs()->EnumObjs(dlgt);
		DWORD c;
		CLevelObj **buf=level->GetIDs()->GetEnumObjs(c);
		_devileyes.resize(c);
		for (int i=0;i<c;i++)
			_devileyes[i]=buf[i]->GetID();
	}

	return;
}


void CBgn_DevilEye_MonitorArea::Update(BGNOutputs &outputs)
{
	CBgp_DevilEye_MonitorArea*pad=_GetPad<CBgp_DevilEye_MonitorArea>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	for (int i=0;i<_devileyes.size();i++)
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(level,_devileyes[i]);
		if (!lo)
			continue;

		CLoGeneralAgent *loAgent=(CLoGeneralAgent *)lo;
		BMO_DevilEyeStatus *status=_GetDevilEyeStatus(loAgent);
		if (!status)
			continue;

		if (status->idTarget!=LevelObjID_Invalid)
		{
			if(lo->GetT()>=status->tTarget)
			{
				CLevelObj *loTarget=LevelUtil_GetAliveLo(level,status->idTarget);
				if (loTarget)
				{
					if(pad->area.CheckIn(loTarget->GetFramePos()))
					{
						_OutputOk(outputs,1,"监测到");
						return;
					}
				}
			}
		}
	}
}
