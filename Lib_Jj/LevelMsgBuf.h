#pragma once

#include "datapacket/BitPacket.h"


class CLevelMsgBuf
{
public:
	void Init()
	{
		_dataFrame.resize(512*1024);
		_bitsFrame.resize(512*1024);
		_bpFrame.SetBufferPointer(_dataFrame.data(),_bitsFrame.data());

		_dataAction.resize(512*1024);
		_bitsAction.resize(512*1024);
		_bpActon.SetBufferPointer(_dataAction.data(),_bitsAction.data());

	}

	CBitPacket *GetFrameBP()	{		return &_bpFrame;	}
	CBitPacket *GetActionBP()	{		return &_bpActon;	}

protected:
	//视野同步消息
	std::vector<BYTE>_dataFrame;
	std::vector<BYTE>_bitsFrame;
	CBitPacket _bpFrame;

	//各种Action的消息
	std::vector<BYTE>_dataAction;
	std::vector<BYTE>_bitsAction;
	CBitPacket _bpActon;

};
