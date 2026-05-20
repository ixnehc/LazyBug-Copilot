#pragma once

#include "BehaviorDebug.h"

#include "../Registry/Registry.h"

class CBehaviorDebugClient_RegistryBase
{
public:
	BOOL Init(const char *nmCompany,const char *nmApp);

	BehaviorDebugState *GetState()	{		return &_state;	}
	BOOL ExistsSelFrameData()	{		return _objSel!=0;	}
    BehaviorDebugFrameData *FindSelFrameData(StringID nmBg)    
	{        
		if (_objSel!=0)
		{
			for (int i=0;i<_framedataSel.size();i++)
			{
				if (nmBg==_framedataSel[i].key.nmBG)
					return &_framedataSel[i];
			}
		}
		return NULL;
	}
	DWORD GetSelObj()	{		return _objSel;	}

	DWORD GetObjCount()	{		return _objs.size();	}
	DWORD GetObj(DWORD idx)	{		return idx<_objs.size()?_objs[idx]:0;	}
	StringID GetObjName(DWORD idx){		return idx<_nmsObj.size()?_nmsObj[idx]:StringID_Invalid;	}
	void GetObjsByName(StringID nm,std::vector<DWORD> &objs);
	StringID GetObjNameByID(DWORD obj);


	void MaintainConnect();

	void Update();
	void SendCommand(BehaviorDebugCmd &cmd);

protected:

	void _ReadState();
    void _ReadFrameData();


	BehaviorDebugState _state;

	std::vector<DWORD> _objs;
	std::vector<StringID> _nmsObj;
	DWORD _objSel;
	std::vector<BehaviorDebugFrameData> _framedataSel;


	CCurrentUserRegistry _reg;

	int _connserial;

	int _cmdserial;
	int _stateserial;
    int _framedataserial;

	std::vector<BYTE>_buf;
};


class CBehaviorDebug_RegistryBase:public CBehaviorDebug
{
public:
	CBehaviorDebug_RegistryBase()
	{
		_connserial=0;
		_cmdserial=0;
		_stateserial=0;
        _framedataserial = 0;
		_bConn=FALSE;
		_tConnSerial=0xffffffffffffffff;
		_bFrameDataChange=FALSE;
	}
	BOOL Init(const char *nmCompany,const char *nmApp);

	void Update();

protected:

	void _MonitorCmd();
	void _UpdateConn();

	virtual void _OnBreakingLoop();
	virtual void _OnStateChange();
    virtual void _OnFrameDataChange()	{		_bFrameDataChange=TRUE;	}
    virtual BOOL _IsConn() { return _bConn; }

	CCurrentUserRegistry _reg;

	int _cmdserial;
	int _stateserial;
    int _framedataserial;

	void _UpdateFrameDataChange();
	BOOL _bFrameDataChange;

	BOOL _bConn;
	int _connserial;
	unsigned __int64 _tConnSerial;//_connserial取得的时间
	std::vector<BYTE>_buf;


};
