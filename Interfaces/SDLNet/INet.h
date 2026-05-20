

#pragma once

#ifdef PROJ_SDLNET_EXPORTS
#define SDLNet_Api __declspec(dllexport)
#else
#define SDLNet_Api __declspec(dllimport)
#endif


typedef DWORD ConnID;
#define ConnID_Invalid 0

//保留的几个消息,用户不要使用它们的值
#define NetMsg_CSConnect 2
#define NetMsg_CSDisconnect 3
#define NetMsg_SCDisconnect 4


struct NetMsg;
struct NetMsgS
{
	ConnID connid;
	NetMsg *msg;
};


class IServerNet
{
public:
	virtual void SendMsg(ConnID id,NetMsg *msg)=0;
	virtual NetMsgS*ReceiveMsgs(DWORD &count)=0;//返回临时指针

	virtual void SetLag(DWORD lag)=0;
};

class IClientNet
{
public:
	virtual BOOL OpenConn(const char *host,WORD port)=0;
	virtual void CloseConn()=0;
	virtual BOOL IsConn()=0;

	virtual void SendMsg(NetMsg *msg)=0;
	virtual NetMsg*ReceiveMsg()=0;//返回临时指针
	virtual BOOL IsAvailableMsg()=0;

	virtual DWORD GetIPHost()=0;

};

SDLNet_Api void InitNet();
SDLNet_Api void UnInitNet();

class CClass;
SDLNet_Api void RegisterNetMsg(CClass *clssMsg);
SDLNet_Api void RegisterNetMsgs(CClass **clssMsgs,DWORD c);

SDLNet_Api IServerNet *CreateServerNet(WORD port);
SDLNet_Api void DestroyServerNet(IServerNet *);

SDLNet_Api IClientNet *CreateClientNet();
SDLNet_Api void DestroyClientNet(IClientNet *);

