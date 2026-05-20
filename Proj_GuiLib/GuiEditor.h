#pragma once

#include "GuiLib.h"

#include "editor/editor.h"
#include "math/rect.h"

#include "GuiCmd.h"



#include <string>

class IRenderPort;

class CGuiView;

class GraphicsGraph;
class IRenderSystem;
class GuiLib_Api CGuiAgent:public CGeAgent
{
public:
	IRenderPort *GetRP();
	IRenderSystem *GetRS();
	GraphicsGraph *GetGG();
	CWnd *GetWnd();
	CGuiView *GetGuiView()	{		return (CGuiView*)GetView();	}
	void GetClientRect(i_math::recti &rc)	{		_GetClientRect(rc);	}

	virtual void OccupyFocus(OpType type);
	virtual void DiscardFocus(OpType type);

	virtual BOOL NeedDepthBuffer()	{		return TRUE;	}

protected:
	//useful operations
	void _AddMenu(const char *name,DWORD id,UINT nFlags=MF_ENABLED|MF_STRING);
	void _AddMenuSep();//seperator
	void _PushMenu(const char *name,UINT nFlags=MF_ENABLED|MF_STRING);
	void _PopMenu();
	CMenu *_GetMenu();
	BOOL _IsInMenu();//返回当前是不是有一个menu在弹出
	void _SetCursor(UINT idCursor,BOOL bImmediately=FALSE);
	void _GetClientRect(i_math::recti &rc);
	void _GetCursorPos(i_math::pos2di &pt);//relative to the client rect
	void _SetCursorPos(i_math::pos2di &pt);//relative to the client rect
	void _SetTransformGG(i_math::pos2df &offGG,i_math::pos2df &scaleGG);
	void _GetTransformGG(i_math::pos2df &offGG,i_math::pos2df &scaleGG);
public://take it as protected
	void _ScreenToGG(int &x,int &y);
	void _GGToScreen(int &x,int &y);
	void _ScreenToGG_f(float &x,float &y);
	void _GGToScreen_f(float &x,float &y);
};


class GraphicsGraph;
class IRenderSystem;
class GuiLib_Api CGuiView:public CGeView
{
public:
	CGuiView()
	{
		Zero();
	}
	~CGuiView()
	{
		Clear();
	}
	void Zero()
	{
		_pRS=NULL;
		_wnd=NULL;
		_rp=NULL;
		_bInMenu=FALSE;

		_bInDraw=FALSE;

		_gg=NULL;

		_dc=NULL;
		_scaleGG.set(1.0f,1.0f);
		_bYInverse=FALSE;
	}
	void Clear();
	void SetRS(IRenderSystem *pRS);
	IRenderSystem *GetRS()	{		return _pRS;	}
	void SetWnd(CWnd *wnd,i_math::recti &rc);
	CWnd *GetWnd()	{		return _wnd;	}
	IRenderPort *GetRP()	{		return _rp;	}
	GraphicsGraph *GetGG()	{		return _gg;	}
	void SetGGYInverse(BOOL bYInverse){		_bYInverse=bYInverse;	}
	void SetTransformGG(i_math::pos2df &off,i_math::pos2df &scale)	{_offGG=off;		_scaleGG=scale;	}
	void GetTransformGG(i_math::pos2df &off,i_math::pos2df &scale)	{		off=_offGG;		scale=_scaleGG;	}
	virtual BOOL Draw();

	//return whether the message are processed,if processed,in ret will return the result
	virtual BOOL RespondMsg(CPoint &ptCursor,UINT msg,WPARAM wParam,LPARAM lParam,LRESULT &ret);
	
	// menu: pointer must be allocated by new .
	CMenu * AddSubMenu(const char *name,DWORD flag=0);
	BOOL PushMenu(CMenu * menu);
	CMenu * PopMenu();
	CMenu * TopMenu();

	void OnDrawMenuItem(LPDRAWITEMSTRUCT param);
	void OnMeasureMenuItem(LPMEASUREITEMSTRUCT param);

protected:

	enum DrawMechanism
	{
		UsingRP,
		UsingDC,
		UsingGG,
	};

	virtual DrawMechanism _GetDrawMechanism()	{		return UsingRP;	}

	virtual void _OnPreDraw(IRenderPort *rp)	{}
	virtual void _OnDraw(IRenderPort *rp)	{}
	virtual void _OnDrawNoDepth(IRenderPort *rp){}
	virtual void _OnDraw(CDC *pDC)	{}
	virtual void _OnDraw(GraphicsGraph *gg)	{	}
	virtual void _OnPostDraw(IRenderPort *rp)	{}
	virtual void _OnPostDraw(CDC *pDC)	{}
	virtual void _OnPostDraw(GraphicsGraph *gg)	{	}

	void _ScreenToGG(int &x,int &y);


	CMenu *_GetMenu();

	void _DrawAgents(BOOL bNoDepth);


	IRenderSystem *_pRS;

	CWnd *_wnd;//target window
	i_math::recti _rc;//relative to the target window's client area
	IRenderPort *_rp;
	CDC *_dc;//只在执行Draw()函数时有效,用于为agent 提供绘制手段
	GraphicsGraph *_gg;
	i_math::pos2df _offGG;
	i_math::pos2df _scaleGG;
	BOOL _bYInverse;


	HCURSOR _hCursor;
	CMenu _menu;
	std::vector<CMenu *> _menustack;  // 
	std::vector<CMenu *> _menuAllocs; // new 分配的空间
	BOOL _bInMenu;

	BOOL _bInDraw;

	friend class CGuiAgent;
};

//////////////////////////////////////////////////////////////////////////
//general-use agents

#define DRAG_BUTTON_LEFT 1
#define DRAG_BUTTON_RIGHT 0
#define DRAG_BUTTON_MIDDLE 2

//Button dragger
template<int T_button,DWORD T_flag,BOOL T_bAnyFlag=FALSE>
class CGuiAgent_Dragger:public CGuiAgent
{
public:
	CGuiAgent_Dragger()
	{
		_bInDrag=FALSE;
	}

	BOOL IsFlagMatched(DWORD flag)
	{
		BOOL bFlagMatch=FALSE;

		if (!T_bAnyFlag)
			bFlagMatch=((T_flag&flag)==T_flag);
		else
			bFlagMatch=((T_flag&flag)!=0);

		return bFlagMatch;
	}
	//Overiding

	virtual BOOL OnLButtonDown(int x,int y,DWORD flag)
	{
		if ((T_button==DRAG_BUTTON_LEFT)&&IsFlagMatched(flag)&&(!_bInDrag))
		{
			_bInDrag=TRUE;
			if (OnBeginDrag(x,y,flag))
			{
				OccupyFocus(OpType_Mouse);
				return FALSE;
			}
			else
				_bInDrag=FALSE;
		}
		return TRUE;
	}
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag)
	{
		if ((T_button==DRAG_BUTTON_LEFT)&&(_bInDrag))
		{
			_bInDrag=FALSE;
			OnEndDrag(x,y,flag);
			DiscardFocus(OpType_Mouse);
			return FALSE;
		}
		return TRUE;
	}
	virtual BOOL OnRButtonDown(int x,int y,DWORD flag)
	{
		if ((T_button==DRAG_BUTTON_RIGHT)&&IsFlagMatched(flag)&&(!_bInDrag))
		{
			_bInDrag=TRUE;
			if (OnBeginDrag(x,y,flag))
			{
				OccupyFocus(OpType_Mouse);
				return FALSE;
			}
			else
				_bInDrag=FALSE;
		}
		return TRUE;
	}
	virtual BOOL OnRButtonUp(int x,int y,DWORD flag)
	{
		if ((T_button==DRAG_BUTTON_RIGHT)&&(_bInDrag))
		{
			_bInDrag=FALSE;
			OnEndDrag(x,y,flag);
			DiscardFocus(OpType_Mouse);
			return FALSE;
		}
		return TRUE;
	}

	virtual BOOL OnMButtonDown(int x,int y,DWORD flag)
	{
		if ((T_button==DRAG_BUTTON_MIDDLE)&&IsFlagMatched(flag)&&(!_bInDrag))
		{
			_bInDrag=TRUE;
			if (OnBeginDrag(x,y,flag))
			{
				OccupyFocus(OpType_Mouse);
				return FALSE;
			}
			else
				_bInDrag=FALSE;
		}
		return TRUE;
	}
	virtual BOOL OnMButtonUp(int x,int y,DWORD flag)
	{
		if ((T_button==DRAG_BUTTON_MIDDLE)&&(_bInDrag))
		{
			_bInDrag=FALSE;
			OnEndDrag(x,y,flag);
			DiscardFocus(OpType_Mouse);
			return FALSE;
		}
		return TRUE;
	}

	virtual BOOL OnMouseMove(int x,int y,DWORD flag)
	{
		if (!_bInDrag)
			return TRUE;
		OnDrag(x,y,flag);
		return FALSE;
	}

	//Overidable
	virtual BOOL OnBeginDrag(int x,int y,DWORD flag)	{return FALSE;}
	virtual void OnEndDrag(int x,int y,DWORD flag)	{}
	virtual void OnDrag(int x,int y,DWORD flag)	{}
protected:
	BOOL _bInDrag;	
};

class GuiLib_Api CGuiActor:public CGeActor
{
public:
	virtual CWnd *GetWnd()	{		return NULL;	}

	virtual void DoCommand(DWORD idCmd);
	virtual void UpdateCommandUI(DWORD idCmd,void *param);

protected:

};

class GuiLib_Api CGuiPanel:public CDialog,public CGuiActor
{
public:
	virtual ~CGuiPanel(){}
	CGuiPanel(UINT id,CWnd *pParent);
	virtual BOOL Create(CWnd *pParent)=0;
	virtual void OnOK()	{	}
	virtual void OnCancel(){	}
	virtual CWnd *GetWnd()	{		 return this;	}


	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


class GuiLib_Api CGuiMgr:public CGeMgr
{
public:
	virtual CGeActor *GetActiveActor();
	virtual void Update();


	void UpdateCommandUI(DWORD idCmd,void *param);
	void DoCommand(DWORD idCmd);


};