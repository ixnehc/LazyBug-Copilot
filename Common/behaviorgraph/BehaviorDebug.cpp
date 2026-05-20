
/********************************************************************
	created:	2014/08/31
	author:		cxi
	
	purpose:	Behavior Debug
*********************************************************************/
#include "stdh.h"
#include "BehaviorDebug.h"

#include "../datapacket/datapacket.h"


//////////////////////////////////////////////////////////////////////////
//BehaviorDebugState
void BehaviorDebugState::Save(CDataPacket &dp)
{
	dp.Data_WriteSimpleR(bpCur);
	dp.Data_WriteSimpleR(objCur);
	DP_WriteVectorN(dp,bps);
}

void BehaviorDebugState::Load(CDataPacket &dp)
{
	dp.Data_ReadSimple(bpCur);
	dp.Data_ReadSimple(objCur);
	DP_ReadVectorN(dp,bps);
}
 
//////////////////////////////////////////////////////////////////////////
//BehaviorDebugFrameData
void BehaviorDebugFrameData::Save(CDataPacket &dp)
{
	dp.Data_WriteSimpleR(key);
    DP_WriteVectorN(dp, pendings);
}

void BehaviorDebugFrameData::Load(CDataPacket &dp)
{
	dp.Data_ReadSimple(key);
    DP_ReadVectorN(dp, pendings);
}




//////////////////////////////////////////////////////////////////////////
//CBehaviorDebug

void CBehaviorDebug::_Break(BehaviorDebugStep &step,DWORD obj)
{
	_state.bpCur=step;
	_state.objCur=obj;

	_OnStateChange();

	while(_state.IsBreaking())
	{
		_OnBreakingLoop();
	}

}


void CBehaviorDebug::Step(BehaviorDebugStep &step,DWORD obj)
{
	_idxStep++;

	if (!_IsConn())
		return;

	if (_objSel!=0)
	{
		if (obj!=_objSel)
			return;
	}

	if (_bNextBreak)
	{
		if (_objNextBreak==obj)
		{
			_bNextBreak=FALSE;
			_Break(step,obj);
			return;
		}
	}

	for (int i=0;i<_state.bps.size();i++)
	{
		if (step.Equal(_state.bps[i]))
		{
			_Break(step,obj);
			break;
		}
	}
}

void CBehaviorDebug::SetFrameData(BehaviorDebugFrameData &dataNew)
{
	BehaviorDebugFrameData &data=_framedatas[dataNew.key.ToValue()];
	data.key=dataNew.key;
	data.pendings=dataNew.pendings;
    _OnFrameDataChange();
}

void CBehaviorDebug::ClearFrameData(DWORD obj,StringID nmBg)
{
	BehaviorDebugFrameData::Key k;
	k.obj=obj;
	k.nmBG=nmBg;
	std::unordered_map<BehaviorDebugFrameData::KeyValue ,BehaviorDebugFrameData>::iterator it=_framedatas.find(k.ToValue());
	if (it!=_framedatas.end())
		_framedatas.erase(it);
// 	if (_objSel==obj)
// 		_objSel=0;
	_OnFrameDataChange();
}




void CBehaviorDebug::HandleCommand(BehaviorDebugCmd &cmd)
{
	switch(cmd.tp)
	{
		case BehaviorDebugCmd::ToggleBreakPoint:
		{
			int idxBP=_state.FindBP(cmd.bp);
			if (idxBP==-1)
				_state.bps.push_back(cmd.bp);
			else
				_state.bps.erase(_state.bps.begin()+idxBP);
			_OnStateChange();
			break;
		}
		case BehaviorDebugCmd::ClearBreakPoint:
		{
			_state.bps.clear();
			_OnStateChange();
			break;
		}
		case BehaviorDebugCmd::StepForward:
		{
			if (_state.IsBreaking())
			{
				_bNextBreak=TRUE;
				_objNextBreak=_state.objCur;
				_state.bpCur.Zero();
				_state.objCur=0;
				_OnStateChange();
			}
			break;
		}
		case BehaviorDebugCmd::Continue:
		{
			if (_state.IsBreaking())
			{
				_bNextBreak=FALSE;
				_state.bpCur.Zero();
				_state.objCur=0;
				_OnStateChange();
			}
			break;
		}
        case BehaviorDebugCmd::SelectObj:
        {
			_objSel=cmd.obj;
			_OnFrameDataChange();
            break;
        }
    }
}
