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
class IClientEntity
{
public:
	virtual GStubConn *FindConn(const char *nm)=0;

	virtual void OnCreate(){};//当_core已经被创建后调用
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
	virtual void DoClock()=0;
	virtual DWORD GetClientEntityDescs(ClientEntityDesc *&descs)=0;
};
