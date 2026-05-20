
/********************************************************************
	created:	2014/08/31
	author:		cxi
	
	purpose:	Behavior Debug (Using registry to communicate)
*********************************************************************/
#include "stdh.h"
#include "BehaviorDebug_RegistryBase.h"


//////////////////////////////////////////////////////////////////////////
//CBehaviorDebugClient_RegistryBase
BOOL CBehaviorDebugClient_RegistryBase::Init(const char *nmCompany,const char *nmApp)
{
	_reg.Init(nmCompany,nmApp);

	_connserial=0;
	_cmdserial=_reg.ReadInt("Debug","CmdSerial");
	_stateserial=_reg.ReadInt("Debug","StateSerial");
    _framedataserial= _reg.ReadInt("Debug", "FrameDataSerial");

	_ReadState();
    _ReadFrameData();

	return TRUE;
}

void CBehaviorDebugClient_RegistryBase::_ReadState()
{
	DWORD szData;
	void *data;
	if (_reg.ReadData("Debug","State",data,szData))
	{
		_buf.resize(szData);
		memcpy(_buf.data(),data,szData);

		CDataPacket dp;
		dp.SetDataBufferPointer(_buf.data());
		_state.Load(dp);
	}
}

void CBehaviorDebugClient_RegistryBase::_ReadFrameData()
{
	_objSel=0;
	_objs.clear();
	_nmsObj.clear();
	_framedataSel.clear();
	_framedataSel.reserve(32);

    DWORD szData;
    void *data;
    if (_reg.ReadData("Debug", "FrameData", data, szData))
    {
        _buf.resize(szData);
        memcpy(_buf.data(), data, szData);

        CDataPacket dp;
        dp.SetDataBufferPointer(_buf.data());

		_objSel=dp.Data_DecodeDword();

		DWORD sz=dp.Data_NextDword();
		_objs.resize(sz);
		_nmsObj.resize(sz);
		for (int i=0;i<sz;i++)
		{
			_objs[i]=dp.Data_DecodeDword();

			if (_objs[i]==_objSel)
			{
				_framedataSel.resize(_framedataSel.size()+1);
				_framedataSel[_framedataSel.size()-1].Load(dp);
				_nmsObj[i]=_framedataSel[_framedataSel.size()-1].key.nmBG;
			}
			else
				_nmsObj[i]=dp.Data_DecodeDword();
		}
    }
}


void CBehaviorDebugClient_RegistryBase::MaintainConnect()
{
	_connserial++;
	_reg.WriteInt("Debug","ConnSerial",_connserial);
}



void CBehaviorDebugClient_RegistryBase::Update()
{
    if (TRUE)
    {
        int stateserialNew = _reg.ReadInt("Debug", "StateSerial");
        if (stateserialNew != _stateserial)
        {
            _ReadState();
        }
        _stateserial = stateserialNew;
    }

    if (TRUE)
    {
        int serialNew = _reg.ReadInt("Debug", "FrameDataSerial");
        if (serialNew != _framedataserial)
        {
            _ReadFrameData();
        }
        _framedataserial = serialNew;
    }

}

void CBehaviorDebugClient_RegistryBase::SendCommand(BehaviorDebugCmd &cmd)
{
	_cmdserial++;

	if (_reg.WriteData("Debug","Cmd",&cmd,sizeof(cmd)))
	{
		_reg.WriteInt("Debug","CmdSerial",_cmdserial);
	}
}

void CBehaviorDebugClient_RegistryBase::GetObjsByName(StringID nm,std::vector<DWORD> &objs)
{
	objs.clear();
	for (int i=0;i<_objs.size();i++)
	{
		if (_nmsObj[i]==nm)
			objs.push_back(_objs[i]);
	}
}


StringID CBehaviorDebugClient_RegistryBase::GetObjNameByID(DWORD obj)
{
	for (int i=0;i<_objs.size();i++)
	{
		if (obj==_objs[i])
			return _nmsObj[i];
	}
	return StringID_Invalid;
}




//////////////////////////////////////////////////////////////////////////
//CBehaviorDebug_RegistryBase

BOOL CBehaviorDebug_RegistryBase::Init(const char *nmCompany,const char *nmApp)
{
	_reg.Init(nmCompany,nmApp);

	_cmdserial=_reg.ReadInt("Debug","CmdSerial");
	_stateserial=_reg.ReadInt("Debug","StateSerial");

	_OnStateChange();//更新一下状态
	_OnFrameDataChange();

	return TRUE;
}

void CBehaviorDebug_RegistryBase::Update()
{
	_MonitorCmd();
	_UpdateConn();

	_UpdateFrameDataChange();
}

void CBehaviorDebug_RegistryBase::_MonitorCmd()
{
	int cmdserialNew=_reg.ReadInt("Debug","CmdSerial");
	if (cmdserialNew!=_cmdserial)
	{//有新的命令
		BehaviorDebugCmd cmd;
		void *data;
		DWORD szData;
		if (_reg.ReadData("Debug","Cmd",data,szData))
		{
			if (data)
			{
				if (szData==sizeof(cmd))
				{
					memcpy(&cmd,data,szData);
					HandleCommand(cmd);
				}
			}
		}
	}
	_cmdserial=cmdserialNew;
}

void CBehaviorDebug_RegistryBase::_UpdateConn()
{
	unsigned __int64 tCur=GetTickCount();

	if (_tConnSerial==0xffffffffffffffff)
	{
		_connserial=_reg.ReadInt("Debug","ConnSerial");
		_tConnSerial=tCur;
		return;
	}

	int connserialNew=_reg.ReadInt("Debug","ConnSerial");
	if (connserialNew!=_connserial)
	{
		_bConn=TRUE;
		_tConnSerial=tCur;
		_connserial=connserialNew;
		return;
	}

	if (_tConnSerial+500<tCur)
	{//一段时间ConnSerial的值没有变化,表示失去连接
		_bConn=FALSE;

		//继续
		BehaviorDebugCmd cmd;
		cmd.tp=BehaviorDebugCmd::Continue;
		HandleCommand(cmd);
	}
}



void CBehaviorDebug_RegistryBase::_OnBreakingLoop()
{
	_MonitorCmd();
	_UpdateConn();
}

void CBehaviorDebug_RegistryBase::_OnStateChange()
{
	_stateserial++;

	_buf.clear();

	DP_BeginSave(dp,_buf);
	_state.Save(dp);
	DP_EndSave();

	_reg.WriteData("Debug","State",_buf.data(),_buf.size());
	_reg.WriteInt("Debug","StateSerial",_stateserial);
}

void CBehaviorDebug_RegistryBase::_UpdateFrameDataChange()
{
	if (!_bFrameDataChange)
		return;

    _framedataserial++;

    _buf.clear();

    DP_BeginSave(dp, _buf);

	if (TRUE)
	{
		dp.Data_EncodeDword(_objSel);

		DWORD sz=_framedatas.size();
		dp.Data_NextDword()=sz;

		std::unordered_map<BehaviorDebugFrameData::KeyValue,BehaviorDebugFrameData>::iterator it;
		for (it=_framedatas.begin();it!=_framedatas.end();it++)
		{
			dp.Data_EncodeDword((*it).second.key.obj);
			if ((*it).second.key.obj==_objSel)
				(*it).second.Save(dp);
			else
				dp.Data_EncodeDword((*it).second.key.nmBG);
		}
	}

    DP_EndSave();

    _reg.WriteData("Debug", "FrameData", _buf.data(), _buf.size());
    _reg.WriteInt("Debug", "FrameDataSerial", _framedataserial);

	_bFrameDataChange=FALSE;
}
