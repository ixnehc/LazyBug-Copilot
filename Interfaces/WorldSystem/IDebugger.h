#pragma once

#include "IEntitySystemDefines.h"
#include "ILuaDefines.h"
#include "fastdelegate/FastDelegate.h"


typedef DWORD BreakID;

struct DebugValue
{
	enum ValueType
	{
		Nil,
		String,
		Number,
		Data,
		Unknown,
	};

	DebugValue()
	{
		vtype=Nil;
	}
	ValueType vtype;

	char key[64];
	char str[64];
};

//代表一个变量的所有描述信息
struct DebugVarDesc
{
	enum Type
	{
		Local,
		Parameter,
		Global,
	};

	Type type;

	std::string path;//变量所在的路径,比如table1.abc.a的路径为table1.abc

	std::vector<DebugValue> values;

};

//代表一个stack的信息
struct DebugStackInfo
{
	DebugStackInfo()
	{
		Zero();
	}
	void Zero()
	{
		protoid=ProtoID_Null;
		nodeid=ProtoNodeID_Null;
		iLine=-1;
		bCPP=FALSE;
	}
	BOOL bCPP;

	ProtoID protoid;
	ProtoNodeID nodeid;
	int iLine;

	std::string nameProto;
	std::string nameProtoNode;
	std::string nameFunc;
	std::string location;
};

enum BreakMode
{
	Break_PointCheck,
	Break_StepOver,
	Break_StepInto,
	Break_TargetLine,
};


struct DebugBreakPoint
{
	DebugBreakPoint()
	{
		line=-1;
		flag=None;
		protoid=ProtoID_Null;
		nodeid=ProtoNodeID_Null;
	}
	enum Flag
	{
		None=0,
		Disable=1,
	};
	ProtoID protoid;
	ProtoNodeID nodeid;
	int line;
	Flag flag;
};




struct DebugOutput
{
	DebugOutput()
	{
		tp=Error;
		protoid=ProtoID_Null;
		nodeid=ProtoNodeID_Null;
		line=-1;
		pathProto=pathProtoNode=content="";
	}
	enum Type
	{
		Error,
		Warning,
		Notify,
	};

	Type tp;
	ProtoID protoid;
	ProtoNodeID nodeid;
	int line;
	const char*pathProto;
	const char *pathProtoNode;
	const char *content;
};
typedef fastdelegate::FastDelegate1<DebugOutput&> DebugOutputHandler;


class IDebugger
{
public:
	virtual void SetWindowInfo(HWND hMainWnd,HACCEL hAccel)=0;

	virtual BOOL Attach(BreakMode mode)=0;
	virtual void Detach()=0;

	virtual BOOL IsBreak()=0;
	virtual BreakID GetBreakID()=0;
	virtual void Continue(BreakMode mode)=0;

	virtual DWORD GetStackCount()=0;
	virtual DebugStackInfo *GetStackInfo(DWORD idx)=0;

	virtual int GetCurStack()=0;
	virtual BOOL SetCurStack(int iStack)=0;

	virtual DebugVarDesc*GetVar(const char *varname)=0;

	virtual DWORD GetBPsVer()=0;
	virtual void ClearAllBreakPoints()=0;
	virtual void UpdateBreakPoints(ProtoID protoid,ProtoNodeID nodeid,DebugBreakPoint *buf,DWORD c)=0;
	virtual DebugBreakPoint *GetBPs(ProtoID protoid,ProtoNodeID nodeid,DWORD &c)=0;

	virtual void SetOutputHandler(DebugOutputHandler dlgt)=0;
	virtual void Output(DebugOutput::Type tp,const char *content)=0;
	virtual void Output(DebugOutput::Type tp,ProtoID idProto,ProtoNodeID idPN,int iLine,const char *content)=0;

	virtual void MakeProtoNames(ProtoID protoid,ProtoNodeID nodeid,const char *&pathProto,const char *&pathNode)=0;

};




struct lua_State;
inline void LuaDebugOutput(lua_State *L,DebugOutput::Type type,const char *content,...)
{
	static char buf[2048];

	IDebugger *dbgr=(IDebugger*)DebuggerFromL(L);

	va_list args;
	va_start(args,content);
	_vsnprintf(buf,sizeof(buf),content,args);
	va_end(args);

	dbgr->Output(type,buf);
}
