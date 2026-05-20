/********************************************************************
	created:	2013/5/25 
	author:		cxi
	
	purpose:	检测与主人间的距离
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"

#include "BgnMonitorMasterLeave.h"

////////////////////////////////////////////////////////////////////////
//CBgn_MonitorMasterLeave
BIND_BGN_CLASS(CBgn_MonitorMasterLeave,CBgp_MonitorMasterLeave);
void CBgn_MonitorMasterLeave::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_MonitorMasterLeave*pad=_GetPad<CBgp_MonitorMasterLeave>();

	CLevelObj *lo=_GetLo();
	extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
	CLevelObj *loOwner=LevelUtil_GetOwnerLo(lo);

	if (lo&&loOwner)
	{
		_posMe=lo->GetFramePos();
		_posMaster=loOwner->GetFramePos();
		_dist=_posMe.getDistanceFrom(_posMaster);

		return;
	}

	_OutputOk(outputs,2,"中止");

}

void CBgn_MonitorMasterLeave::Update(BGNOutputs &outputs)
{
	CBgp_MonitorMasterLeave*pad=_GetPad<CBgp_MonitorMasterLeave>();

	CLevelObj *lo=_GetLo();
	extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
	CLevelObj *loOwner=LevelUtil_GetOwnerLo(lo);

	if (lo&&loOwner)
	{
		LevelPos posMe,posMaster;
		posMe=lo->GetFramePos();
		posMaster=loOwner->GetFramePos();

		if (posMaster.getDistanceSQFrom(posMe)>pad->radius*pad->radius)
		{
			float dist=posMaster.getDistanceFrom(_posMe);
			float dDist=dist-_dist;
			_distLeave+=dDist; 
			if (_distLeave<0.0f)
				_distLeave=0.0f;
		}
		else
			_distLeave=0.0f;

		_posMaster=posMaster;
		_posMe=_posMe;
		_dist=_posMe.getDistanceFrom(_posMaster);

		if (_distLeave>3.0)
		{
			_OutputOk(outputs,1,"远离");
		}
		return;
	}

	_OutputOk(outputs,2,"中止");

}
