#pragma once

#include "GuiLib.h"

#include "editor/editor.h"

#include "WorldSystem/IEntitySystem.h"
#include "RenderSystem/IRenderSystem.h"


//ProtoLibFrame主窗口的控制接口
class CPrlFrameProxy
{
public:
	virtual void EnableAllLuaSrc(BOOL bEnable)=0;
	virtual void UpdateLuaSrcToProto(IProto *proto)=0;
	virtual void GotoLuaSrc(ProtoID protoid,ProtoNodeID nodeid,int iLine=0)=0;
	virtual int FindLuaSrcFunc(ProtoID protoid,ProtoNodeID nodeid,const char *nameFunc)=0;
	virtual int FindLuaSrcVar(ProtoID protoid,ProtoNodeID nodeid,const char *nameVar)=0;
	virtual void AddLuaSrcFunc(ProtoID protoid,ProtoNodeID nodeid,const char *nameFunc)=0;
	virtual BOOL GotoAppearance(ProtoID protoid)=0;
	virtual BOOL GotoLogic(ProtoID protoid)=0;
	virtual void ClearDebugOutput()=0;
	virtual void ShowLuaHelp(BOOL bShow)=0;
	virtual void SetHelpKey(const char *func)=0;
	virtual const char *GetStartMain()=0;//得到运行起始的proto
	virtual ProtoID GetActiveProto()=0;
	virtual BOOL IsProtoOpened(ProtoID protoid)=0;
};


struct GuiData_PrlFrameProxy:public GeData
{
	virtual const char *GetName()	{		return "prlframeproxy";	}
	GuiData_PrlFrameProxy()
	{
		proxy=NULL;
	}

	CPrlFrameProxy *proxy;
};

