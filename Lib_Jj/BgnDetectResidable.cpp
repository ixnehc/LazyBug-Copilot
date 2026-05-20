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

#include "BgnDetectResidable.h"


////////////////////////////////////////////////////////////////////////
//CBgn_DetectResidable

BIND_BGN_CLASS(CBgn_DetectResidable,CBgp_DetectResidable);

void CBgn_DetectResidable::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_DetectResidable*pad=_GetPad<CBgp_DetectResidable>();

	CLevelObj *loDetect=NULL;
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		extern CLevelObj *LevelUtil_DetectClosestResidable(CLevelObj *lo,float range,CLevelObj *toIgnore,RecordID idAgent);
		loDetect=LevelUtil_DetectClosestResidable(lo,pad->_radius,NULL,pad->_idAgent);
	}

	if (loDetect)
	{
		if (pad->_nmVar!=StringID_Invalid)
			_SetID(pad->_nmVar,BehaviorMemType_ObjID,loDetect->GetID());
		_OutputOk(outputs,1,"侦测到");
	}
	else
	{
		if (pad->_nmVar!=StringID_Invalid)
			_SetID(pad->_nmVar,BehaviorMemType_ObjID,LevelObjID_Invalid);
		_OutputFail(outputs,2,"未侦测到");
	}
}

