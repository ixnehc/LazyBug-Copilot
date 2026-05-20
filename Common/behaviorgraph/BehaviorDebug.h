#pragma once

#include "../class/class.h"
#include "../strlib/strlibdefines.h"
#include "../linkpad/LinkPadDefines.h"

#include <unordered_map>

#include "BehaviorDefines.h"

struct BehaviorDebugStep
{
	BehaviorDebugStep()
	{
		Zero();
	}
	void Zero()
	{
		nmBG=StringID_Invalid;
		idPad=PadID_Null;
		bBreaking=FALSE;
		result=A_Pending;
	}
	BOOL IsEmpty()	{		return nmBG==StringID_Invalid;	}
	BOOL Equal(BehaviorDebugStep &stepOther)
	{
		return (nmBG==stepOther.nmBG)&&(idPad==stepOther.idPad);
	}

	StringID nmBG;//behavior graph的名字
	PadID idPad;
	BOOL bBreaking;
	AResult result;
};

struct BehaviorDebugCmd
{
	enum Type
	{
		None,
		ToggleBreakPoint,
		ClearBreakPoint,
		StepForward,
		Continue,
        SelectObj,
	};

	BehaviorDebugCmd()
	{
		tp=None;
        obj = 0;
	}

	Type tp;

	//用来SetBreakPoint/RemoveBreakPoint
	BehaviorDebugStep bp;

    DWORD obj;

};

struct BehaviorDebugState
{
	BOOL IsBreaking()
	{
		return !bpCur.IsEmpty();
	}
	int FindBP(BehaviorDebugStep &step)
	{
		for (int i=0;i<bps.size();i++)
		{
			if (bps[i].Equal(step))
				return i;
		}
		return -1;
	}
	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);
	DWORD objCur;//当前断在那个obj里
	BehaviorDebugStep bpCur;//当前断在哪里
	std::vector<BehaviorDebugStep> bps;//所有设置的断点
};

struct BehaviorDebugFrameData
{
    BehaviorDebugFrameData()
    {
		Zero();
    }

    void Zero()
    {
        key.nmBG = StringID_Invalid;
		key.obj=0;
    }

    void Clear()
    {
        pendings.clear();
        Zero();
    }

    void Save(CDataPacket &dp);
    void Load(CDataPacket &dp);

	typedef unsigned __int64 KeyValue;

	struct Key
	{
		KeyValue ToValue()
		{
			unsigned __int64 v;
			v=obj;
			v<<=32;
			v|=(unsigned __int64)nmBG;
			return v;
		}
		DWORD obj;
		StringID nmBG;
	};

	Key key;
    std::vector<PadID> pendings;
};


class CBehaviorDebug
{
public:
	CBehaviorDebug()
	{
		Zero();
	}
	~CBehaviorDebug()
	{
		Clear();
	}
	void Zero()
	{
		_idxStep=0;
		_bNextBreak=FALSE;
		_objNextBreak=0;
		_objSel=0;
	}

	void Clear()
	{
		_framedatas.clear();
		Zero();
	}

	void HandleCommand(BehaviorDebugCmd &cmd);

	void Step(BehaviorDebugStep &step,DWORD obj);

    void SetFrameData(BehaviorDebugFrameData &data);
	void ClearFrameData(DWORD obj,StringID nmBg);

protected:

	void _Break(BehaviorDebugStep &step,DWORD obj);

	virtual void _OnBreakingLoop()=0;
	virtual void _OnStateChange()=0;
    virtual void _OnFrameDataChange() = 0;
    virtual BOOL _IsConn() = 0;


	int _idxStep;
	BOOL _bNextBreak;//下一次step中断
	DWORD _objNextBreak;
	BehaviorDebugState _state;

	DWORD _objSel;
	std::unordered_map<BehaviorDebugFrameData::KeyValue,BehaviorDebugFrameData> _framedatas;
};
