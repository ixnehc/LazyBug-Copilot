#pragma once
#include <vector>
#include <string>

#include "GuiLibDefines.h"

class IWnd;
class IGuiLib
{
public:
	virtual IWnd *NewWnd(GuiWndType tp)=0;
};

class IWndCallBack
{
};

class IWnd//a tree control to show a group of ResData
{
public:
	virtual BOOL SetCallBack(IWndCallBack *callback)=0;
	virtual BOOL SetContent(const char *name,const void *content)=0;
	virtual const void *GetContent(const char *name)=0;
	virtual BOOL SetStyle(GuiWndStyle style)=0;
	virtual BOOL Create(HWND hWnd,RECT &rc,UINT id)=0;//create window
	virtual void Destroy()=0;//destroy window
	virtual BOOL Attach(HWND hWnd)=0;
	virtual BOOL Detach()=0;
	virtual void Delete()=0;//delete this interface
};

class IResTreeCallBack:public IWndCallBack
{
};

class IResTree:public IWnd
{
public:
};