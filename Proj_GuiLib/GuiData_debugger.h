#pragma once

#include "GuiLib.h"

#include "editor/editor.h"

#include "WorldSystem/IEntitySystem.h"
#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IDebugger.h"

#define DEFINE_GUIDATA_DEBUGGER(v)												\
	GuiData_Debugger *v=(GuiData_Debugger*)FindData("debugger");	

#define MIN_DEBUG_ACC (-7)


struct GuiLib_Api DebuggerContext
{
	DebuggerContext()
	{
		bRunning=FALSE;
		pES=NULL;
		dbgr=NULL;
		entity=NULL;
		protoid=ProtoID_Null;
		bNeedStop=FALSE;
		bRecentProgress=FALSE;
		bClient=FALSE;

		acc=0;
	}

	BOOL IsRunning();
	BOOL IsBreak();

	void Run(BreakMode mode,ProtoID idProto,ProtoID idGE,ProtoID idGT,BOOL bAllowEditHelper);
	void Continue(BreakMode mode);
	void RequestStop();
	void Attach(BreakMode mode);
	void Detach();
	void TogglePause();
	void StepPause();
	BOOL IsPaused();
	void Resume(DWORD nFrames);
	void Pause();

	void SetProgressDrawCallBack(ESProgressCallBack dlgt)	{		dlgtProgressDraw=dlgt;	}

	void AddOp(CtrlOp &op)	{		input.AddOp(op);	}
	void SetRPSize(i_math::size2di &sz)	{		input.SetRPSize(sz);	}
	void SetCursorPos(int x,int y)	{		input.SetCursorPos(x,y);	}
	void Update();

	BOOL OnProgress();

	BOOL bRunning;

	BOOL bRecentProgress;

	IEntitySystem *pES;

	IDebugger *dbgr;

	ProtoID protoid;
	IEntity *entity;//如果entity不为空,则处在运行模式下

	DWORD tLast;
	EntitySystemInput input;

	int acc;

	BOOL bClient;

	ESProgressCallBack dlgtProgressDraw;

	BOOL bNeedStop;
};


class IDebugger;
struct GuiLib_Api GuiData_Debugger:public GeData
{
	virtual const char *GetName()	{		return "debugger";	}


	GuiData_Debugger()
	{
		context=NULL;
		verHelp=0;
	}

	void Clear()
	{
		context=NULL;
	}

	DebuggerContext*context;

	void SetHelpKey(const char *key)
	{
		keyHelp=key;
		verHelp++;
	}


	DWORD verHelp;
	std::string keyHelp;

};
