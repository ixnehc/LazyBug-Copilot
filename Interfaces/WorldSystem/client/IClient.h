/********************************************************************
	created:	2011/6/27   15:03
	file path:	e:\IxEngine\Interfaces\GameSystem
	author:		chenxi
	
	purpose:	客户端需要暴露的接口
*********************************************************************/
#pragma once






class CClass;
struct ClientEntityDesc
{
	const char *pathProto;
	CClass *clss;
};


struct GStubConn;
class IEntity;
struct GStubBase;
class IClientEntity
{
public:
	virtual CClass *GetClass()=0;
	virtual GStubConn *GetConn(int idx)=0;
	virtual DWORD GetStubCount()=0;
	virtual GStubBase *GetStub(DWORD idx)=0;

	virtual void OnCreate(){};//当_core已经被Create后调用
	virtual void OnPostCreate(){};//当_core已经被PostCreate后调用
	virtual void OnDestroy(){};//当_core将要被Destroy前调用

public://take it as protected
	IEntity *_core;

};

struct EntitySystemState;
class IClient
{
public:
	virtual BOOL Init(EntitySystemState *ss)=0;
	virtual void UnInit()=0;
	virtual BOOL Attach()=0;
	virtual void Detach()=0;
	virtual BOOL IsAttached()=0;

	virtual void SetCmdLine(const char *strCmdLine)=0;

	virtual void DoClock()=0;
	virtual void DoClock_Paused()=0;
	virtual DWORD GetClientEntityDescs(ClientEntityDesc *&descs)=0;

	virtual void Pause()=0;
	virtual void Resume(DWORD nFrames)=0;

	virtual const char *GetGE()=0;
	virtual const char *GetGT()=0;
};
