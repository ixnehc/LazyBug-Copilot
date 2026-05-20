/********************************************************************
	created:	2008/2/20   14:44
	file path:	d:\IxEngine\Proj_GuiLib
	author:		cxi
	
	purpose:	base class for all the gui editor components
*********************************************************************/
#include "stdh.h"

#include "GuiEditor.h"

#include "RenderSystem/IRenderPort.h"

 
#include <vector>
#include <string>

#include "stringparser/stringparser.h" 

#include "GuiEditor.h"

#include "RenderSystem/IRenderSystem.h"
#include ".\guieditor.h"

#include "WndBase.h"

#include "graphicsgraph.h"


#include "timer/profiler.h"


//////////////////////////////////////////////////////////////////////////
//CGuiAgent
IRenderPort *CGuiAgent::GetRP()
{
	if (!GetGuiView())
		return NULL;
	return GetGuiView()->GetRP();
}

IRenderSystem *CGuiAgent::GetRS()
{
	if (!GetGuiView())
		return NULL;
	return GetGuiView()->GetRS();
}


GraphicsGraph *CGuiAgent::GetGG()
{
	if (!GetGuiView())
		return NULL;
	return GetGuiView()->GetGG();

}

CWnd *CGuiAgent::GetWnd()
{
	if (!GetGuiView())
		return NULL;
	return GetGuiView()->GetWnd();
}


void CGuiAgent::_AddMenu(const char *name,DWORD id,UINT nFlags)
{
	CMenu *menu=GetGuiView()->_GetMenu();
	menu->AppendMenu(nFlags, id, fromMBCS(name));
}

CMenu * CGuiAgent::_GetMenu()
{
	return GetGuiView()->TopMenu();	
}

void CGuiAgent::_AddMenuSep()//seperator
{
	CMenu *menu=GetGuiView()->_GetMenu();
	menu->AppendMenu(MF_BYPOSITION | MF_SEPARATOR, 0, _T(""));
}

void CGuiAgent::_PushMenu(const char *name,UINT nFlags)
{
	CMenu *menu=GetGuiView()->AddSubMenu(name,nFlags);
	GetGuiView()->PushMenu(menu);
}

void CGuiAgent::_PopMenu()
{
	GetGuiView()->PopMenu();
}


//返回当前是不是有一个menu在弹出
BOOL CGuiAgent::_IsInMenu()
{
	return GetGuiView()->_bInMenu;
}



void CGuiAgent::_SetCursor(UINT idCursor,BOOL bImmediately)
{
	if (!GetGuiView())
		return;
	extern HINSTANCE g_hInstance;
	HCURSOR h=(HCURSOR)::LoadImage(g_hInstance,MAKEINTRESOURCE(idCursor),
										IMAGE_CURSOR,0,0,LR_SHARED);
	if (!bImmediately)
		GetGuiView()->_hCursor=h;
	else
		::SetCursor(h);
}

void CGuiAgent::_GetClientRect(i_math::recti &rc)
{
	rc.set(0,0,0,0);
	if (!GetGuiView())
		return;
	rc=GetGuiView()->_rc;
	rc.zeroBase();
}

void CGuiAgent::_GetCursorPos(i_math::pos2di &pt)
{
	pt.set(0,0);
	if (!GetGuiView())
		return;
	if (!GetGuiView()->GetWnd())
		return;

	CPoint pt2;
	GetCursorPos(&pt2);
	GetGuiView()->GetWnd()->ScreenToClient(&pt2);

	pt.x=pt2.x-GetGuiView()->_rc.Left();
	pt.y=pt2.y-GetGuiView()->_rc.Top();
}

void CGuiAgent::_SetCursorPos(i_math::pos2di &pt)
{
	if (!GetGuiView())
		return;
	if (!GetGuiView()->GetWnd())
		return;

	CPoint pt2;
	pt2.x=pt.x+GetGuiView()->_rc.Left();
	pt2.y=pt.y+GetGuiView()->_rc.Top();

	GetGuiView()->GetWnd()->ClientToScreen(&pt2);
	SetCursorPos(pt2.x,pt2.y);
}

void CGuiAgent::_SetTransformGG(i_math::pos2df &offGG,i_math::pos2df &scaleGG)
{
	if (GetGuiView())
		GetGuiView()->SetTransformGG(offGG,scaleGG);
}

void CGuiAgent::_GetTransformGG(i_math::pos2df &offGG,i_math::pos2df &scaleGG)
{
	if (GetGuiView())
		GetGuiView()->GetTransformGG(offGG,scaleGG);
}


//World to Screen
void CGuiAgent::_GGToScreen(int &x,int &y)
{
	CGuiView *view=GetGuiView();
	if (!view)
		return;

	x=(int)(((float)x)*view->_scaleGG.x+view->_offGG.x);
	if (!view->_bYInverse)
		y=(int)(((float)y)*view->_scaleGG.y+view->_offGG.y);
	else
		y=(int)(((float)y)*(-view->_scaleGG.y)+view->_offGG.y);
}

// Screen to World
void CGuiAgent::_ScreenToGG(int &x,int &y)
{
	CGuiView *view=GetGuiView();
	if (!view)
		return;
	view->_ScreenToGG(x,y);
}

void CGuiAgent::_GGToScreen_f(float &x,float &y)
{
	CGuiView *view=GetGuiView();
	if (!view)
		return;

	x=(float)(((float)x)*view->_scaleGG.x+view->_offGG.x);
	if (!view->_bYInverse)
		y=(float)(((float)y)*view->_scaleGG.y+view->_offGG.y);
	else
		y=(float)(((float)y)*(-view->_scaleGG.y)+view->_offGG.y);
}

void CGuiAgent::_ScreenToGG_f(float &x,float &y)
{
	CGuiView *view=GetGuiView();
	if (!view)
		return;
	x=(float)((((float)x)-view->_offGG.x)/view->_scaleGG.x);
	if (!view->_bYInverse)
		y=(float)((((float)y)-view->_offGG.y)/view->_scaleGG.y);
	else
		y=(float)((((float)y)-view->_offGG.y)/(-view->_scaleGG.y));
}


void CGuiAgent::OccupyFocus(OpType type)
{
	CGeAgent::OccupyFocus(type);
	if (type==OpType_Mouse)
	{
		if (GetWnd())
			GetWnd()->SetCapture();
	}
}
void CGuiAgent::DiscardFocus(OpType type)
{
	if (type==OpType_Mouse)
	{
		if (_GetFocus(OpType_Mouse)==this)
			ReleaseCapture();
	}

	CGeAgent::DiscardFocus(type);
}


//////////////////////////////////////////////////////////////////////////
//CGuiView

void CGuiView::Clear()
{
	SAFE_RELEASE(_rp);
	if (_gg)
	{
		_gg->Destroy();
		SAFE_DELETE(_gg);
	}
	Zero();
}



#define CLICK_THRESHOLD 2

GuiLib_Api void CtrlOpFromMsg(CtrlOp *op,DWORD nOp,POINT &ptCursor,UINT msg,WPARAM wParam,LPARAM lParam)
{
	POINT pt=ptCursor;
	BOOL bCtrlDown,bShiftDown,bAltDown;
	bCtrlDown=((GetKeyState(VK_LCONTROL)&0x80)!=0)||((GetKeyState(VK_RCONTROL)&0x80)!=0);
	bShiftDown=((GetKeyState(VK_LSHIFT)&0x80)!=0)||((GetKeyState(VK_RSHIFT)&0x80)!=0);
	bAltDown=((GetKeyState(VK_LMENU)&0x80)!=0)||((GetKeyState(VK_LMENU)&0x80)!=0);

	memset(op,0,nOp*sizeof(CtrlOp));
	for (int i=0;i<nOp;i++)
	{
		op[i].x=pt.x;
		op[i].y=pt.y;
		if (bCtrlDown)
			op[i].flag|=CtrlOpFlag_CtrlDown;
		if (bShiftDown)
			op[i].flag|=CtrlOpFlag_ShiftDown;
		if (bAltDown)
			op[i].flag|=CtrlOpFlag_AltDown;
	}

	static BOOL bClickDown=FALSE;
	static BOOL bLBDown;//if bClickDown is TRUE,indicate which button is down
	static POINT ptClickDown;

	switch(msg)
	{
		case WM_MOUSEWHEEL:
		{
			op[0].op=CtrlOp::Op_Wheel;
			op[0].delta=((short) (((DWORD)(wParam) >> 16) & 0xFFFF));
			break;
		}
		case WM_MOUSEMOVE:
		{
			op[0].op=CtrlOp::Op_Move;
			break;
		}
		case WM_LBUTTONDOWN:
		{
			op[0].vk=VK_LBUTTON;
			op[0].op=CtrlOp::Op_Down;
			bClickDown=TRUE;
			bLBDown=TRUE;
			ptClickDown.x=pt.x;
			ptClickDown.y=pt.y;
			break;
		}
		case WM_LBUTTONUP:
		{
			op[0].vk=VK_LBUTTON;
			op[0].op=CtrlOp::Op_Up;
			if ((bClickDown)&&(bLBDown))
			{
				if ((abs(ptClickDown.x-pt.x)<CLICK_THRESHOLD)&&
					(abs(ptClickDown.y-pt.y)<CLICK_THRESHOLD))
				{
					op[1].vk=VK_LBUTTON;
					op[1].op=CtrlOp::Op_Click;
				}
			}
			bClickDown=FALSE;
			break;
		}
		case WM_LBUTTONDBLCLK:
		{
			op[0].vk=VK_LBUTTON;
			op[0].op=CtrlOp::Op_DblClick;
			break;
		}
		case WM_RBUTTONDOWN:
		{
			op[0].vk=VK_RBUTTON;
			op[0].op=CtrlOp::Op_Down;
			bClickDown=TRUE;
			bLBDown=FALSE;
			ptClickDown.x=pt.x;
			ptClickDown.y=pt.y;
			break;
		}
		case WM_RBUTTONUP:
		{
			op[0].vk=VK_RBUTTON;
			op[0].op=CtrlOp::Op_Up;
			if ((bClickDown)&&(!bLBDown))
			{
				if ((abs(ptClickDown.x-pt.x)<CLICK_THRESHOLD)&&
					(abs(ptClickDown.y-pt.y)<CLICK_THRESHOLD))
				{
					op[1].vk=VK_RBUTTON;
					op[1].op=CtrlOp::Op_Click;
				}
			}
			bClickDown=FALSE;
			break;
		}
		case WM_RBUTTONDBLCLK:
		{
			op[0].vk=VK_RBUTTON;
			op[0].op=CtrlOp::Op_DblClick;
			break;
		}

		case WM_MBUTTONDOWN:
		{
			op[0].vk=VK_MBUTTON;
			op[0].op=CtrlOp::Op_Down;
			bClickDown=TRUE;
			bLBDown=FALSE;
			ptClickDown.x=pt.x;
			ptClickDown.y=pt.y;
			break;
		}
		case WM_MBUTTONUP:
		{
			op[0].vk=VK_MBUTTON;
			op[0].op=CtrlOp::Op_Up;
			if ((bClickDown)&&(!bLBDown))
			{
				if ((abs(ptClickDown.x-pt.x)<CLICK_THRESHOLD)&&
					(abs(ptClickDown.y-pt.y)<CLICK_THRESHOLD))
				{
					op[1].vk=VK_MBUTTON;
					op[1].op=CtrlOp::Op_Click;
				}
			}
			bClickDown=FALSE;
			break;
		}
		case WM_MBUTTONDBLCLK:
		{
			op[0].vk=VK_MBUTTON;
			op[0].op=CtrlOp::Op_DblClick;
			break;
		}

		case WM_KEYDOWN:
		{
			if (wParam!=VK_PROCESSKEY)
			{
				op[0].vk=(int)((wParam)&0xFF);
				op[0].ch=-1;
				op[0].op=CtrlOp::Op_Down;
			}
			break;
		}
		case WM_KEYUP:
		{
			if (wParam!=VK_PROCESSKEY)
			{
				op[0].vk=(int)((wParam)&0xFF);
				op[0].ch=-1;
				op[0].op=CtrlOp::Op_Up;
			}
			break;
		}
		case WM_CHAR:
		{
			static BOOL bLeadingByte=FALSE;
			static char cLeadingByte[2];
			if (!bLeadingByte)
			{
				WORD c;
				c=(WORD)(wParam);
				if (IsDBCSLeadByte((BYTE)c))
				{
					cLeadingByte[0]=(char)c;
					bLeadingByte=TRUE;
					return;
				}
				else
				{
					op[0].vk=-1;
					op[0].ch=c;
					op[0].op=CtrlOp::Op_Char;
				}
			}
			else
			{
				cLeadingByte[1]=(char)(wParam);
				bLeadingByte=FALSE;

				op[0].vk=-1;
				op[0].ch=*(WORD*)cLeadingByte;
				op[0].op=CtrlOp::Op_Char;
			}
			if (op[0].ch<32)
			{
				op[0].op=CtrlOp::Op_None;
				return;
			}
			break;
		}
		case WM_TIMER:
		{
			op[0].op=CtrlOp::Op_Timer;
			op[0].dt=0;
			break;
		}
		case WM_SETCURSOR:
		{
			op[0].op=CtrlOp::Op_SetCursor;
			break;
		}
		case WM_COMMAND:
		{
			op[0].op=CtrlOp::Op_Cmd;
			op[0].idCmd=LOWORD(wParam);
			break;
		}
	}

}


CMenu *CGuiView::_GetMenu()
{
	return TopMenu();
}
BOOL CGuiView::PushMenu(CMenu * menu)
{
	assert(menu);
	for(int i = 0;i<_menustack.size();i++){
		if(_menustack[i]==menu)
			return FALSE;
	}
	_menustack.push_back(menu);
	return TRUE;
}
CMenu * CGuiView::PopMenu()
{
	CMenu * topMenu = TopMenu();
	if(topMenu)
		_menustack.pop_back();
	return topMenu;
}
CMenu * CGuiView::TopMenu()
{
	if(0==_menustack.size())
		return NULL;
	return _menustack.back();
}
CMenu * CGuiView::AddSubMenu(const char *name,DWORD flag/*= 0*/)
{
	CMenu * menu = new CMenu();
	BOOL bOK = FALSE;
	if(menu){
		menu->CreateMenu();
		assert(menu);
		_menuAllocs.push_back(menu);
		CMenu * pParent = TopMenu();
		bOK = pParent->AppendMenu(MF_POPUP | MF_ENABLED | MF_STRING | flag, (UINT_PTR)menu->m_hMenu, fromMBCS(name));
	}
	return menu;
}
BOOL CGuiView::RespondMsg(CPoint &ptCursor,UINT msg,WPARAM wParam,LPARAM lParam,LRESULT &ret)
{
	CtrlOp op[4];
	CtrlOpFromMsg(op,4,FORCE_TYPE(POINT,ptCursor),msg,wParam,lParam);

	if(msg==WM_CAPTURECHANGED)
	{
		if (GetFocus(OpType_Mouse))
			GetFocus(OpType_Mouse)->DiscardFocus(OpType_Mouse);
		return FALSE;
	}

	int ii=0;
	while(op[ii].op!=CtrlOp::Op_None)//Handle it
	{
		if (!_bInMenu)
		{
			_menu.DestroyMenu();
			_menu.CreatePopupMenu();//empty the menu
			
			//Init menu stack
			if(TRUE){
				for(int i = 0;i<_menuAllocs.size();i++){
					SAFE_DELETE(_menuAllocs[i]);
				}
				_menustack.clear();
				_menustack.push_back(&_menu);
			}
		}

		_hCursor=NULL;

		if (!_bInMenu)
			Respond(op[ii]);
		else
		{
			if (op[ii].op==CtrlOp::Op_Cmd)
				Respond(op[ii]);
		}

		if (!_bInMenu)
		{
			if (_menu.GetMenuItemCount()>0)
			{
				POINT pt;
				GetCursorPos(&pt);
	//			_wnd->ScreenToClient(&pt);

				_bInMenu=TRUE;
	//			XTFuncContextMenu(&_menu,TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,_wnd,0);
				_menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,_wnd,NULL);
				_bInMenu=FALSE;

			}
		}

		if (_hCursor)
		{
			if (_wnd)
			{
				if ((::GetFocus()==_wnd->m_hWnd)&&(!_bInMenu))
				{
					CRect rc;
					GetWndClippedRect(_wnd,rc,FALSE);
					CPoint ptCursor;
					GetCursorPos(&ptCursor);
					if (rc.PtInRect(ptCursor))
					{
						SetCursor(_hCursor);
						if (msg==WM_SETCURSOR)
						{
							ret=0;
							return TRUE;
						}
					}
				}
			}

		}
		ii++;
	}
	return FALSE;
}

void CGuiView::SetRS(IRenderSystem *pRS)
{		
	_pRS=pRS;	
	SAFE_RELEASE(_rp);
	_rp=pRS->CreateRenderPort();
}


void CGuiView::SetWnd(CWnd *wnd,i_math::recti &rc)	
{		
	if ((_wnd!=wnd)||(_rc!=rc))
		Invalidate();
	_wnd=wnd;
	_rc=rc;
}

void CGuiView::_DrawAgents(BOOL bNoDepth)
{
	if (_states.size()<=0)
		return;
// 	for (int i=0;i<_CurState()->nAgents;i++)
	for (int i=_CurState()->nAgents-1;i>=0;i--)
	{
		CGeAgent *agent=_CurState()->agents[i].agent;
		if ((!bNoDepth)&&(!((CGuiAgent*)agent)->NeedDepthBuffer()))
			continue;
		if (bNoDepth&&(((CGuiAgent*)agent)->NeedDepthBuffer()))
			continue;
		if (agent->IsEnable())
			if (FALSE==agent->OnDraw())
				return;
	}
}


BOOL CGuiView::Draw()
{
	if (_bInDraw)
		return TRUE;
	if (!_wnd)
		return FALSE;
	if (!_rc.isValid())
		return FALSE;

	if (!_wnd->IsWindowVisible())
		return TRUE;

	_bInDraw=TRUE;

	if (_GetDrawMechanism()==UsingRP)
	{
		if ((_rp)&&(_pRS))
		{
			_rp->SetRect(_rc.Left(),_rc.Top(),_rc.Right(),_rc.Bottom());

			_pRS->BeginFrame();

			_OnPreDraw(_rp);

			_OnDraw(_rp);//绘制view里面需要depth buffer的部分
			_DrawAgents(FALSE);//绘制所有需要depth buffer的agents
			_OnDrawNoDepth(_rp);//绘制view里面不需要depth的部分
			_DrawAgents(TRUE);//绘制所有不需要depth buffer的agents

			_OnPostDraw(_rp);

			_pRS->EndFrame();

			CRect rc,rc2,rcView;
			extern BOOL GetWndClippedRect(CWnd *pWnd,CRect &rc,BOOL bConsiderChildren);
			if (TRUE)
			{
				CWnd *wnd=_wnd;
				rc.SetRect(0,0,10000,10000);
				while(wnd)
				{
					if (wnd==_wnd)
						GetWndClippedRect(wnd,rc2,TRUE);
					else
						GetWndClippedRect(wnd,rc2,FALSE);
					rc=rc&rc2;
					if (wnd->GetStyle()&WS_POPUP)
						break;
					wnd=wnd->GetParent();
				}
			}
			_wnd->GetWindowRect(&rc2);
			rcView.IntersectRect(&rc,&rc2);
			_wnd->ScreenToClient(&rcView);

//			ProfilerStart(Present);
			_pRS->Present((i_math::recti*)&rcView,(i_math::recti*)&rcView,_wnd->m_hWnd);
//			ProfilerEnd();

		}
	}

	if (_GetDrawMechanism()==UsingDC)
	{
		_dc=_wnd->GetDC();
	
		_OnDraw(_dc);
		CGeView::Draw();//let the agents draw
		_OnPostDraw(_dc);

		_wnd->ReleaseDC(_dc);
		_dc=NULL;
	}

	if (_GetDrawMechanism()==UsingGG)
	{
		CDC *pDC=_wnd->GetDC();

		if (!_gg)
			_gg=new GraphicsGraph;

		Gdiplus::Graphics grph(pDC->m_hDC);


		_gg->Create(_rc.getWidth(),_rc.getHeight());

		_gg->Transform(_offGG,_scaleGG,_bYInverse);

		_OnDraw(_gg);
		CGeView::Draw();//let the agents draw
		_OnPostDraw(_gg);

		grph.DrawImage(_gg->GetBg(), Gdiplus::Point(_rc.Left(),_rc.Top()));

		_wnd->ReleaseDC(pDC);
	}

	_bInDraw=FALSE;

	return TRUE;
}

static CMenu* FindPopupMenuFromID(CMenu* pMenu, UINT nID)
{
	ASSERT_VALID(pMenu);
	// walk through all items, looking for ID match
	UINT nItems = pMenu->GetMenuItemCount();
	for (int iItem = 0; iItem < (int)nItems; iItem++)
	{
		CMenu* pPopup = pMenu->GetSubMenu(iItem);
		if (pPopup != NULL)
		{
			// recurse to child popup
			pPopup = FindPopupMenuFromID(pPopup, nID);
			// check popups on this popup
			if (pPopup != NULL)
				return pPopup;
		}
		else if (pMenu->GetMenuItemID(iItem) == nID)
		{
			// it is a normal item inside our popup
			pMenu = CMenu::FromHandlePermanent(pMenu->m_hMenu);
			return pMenu;
		}
	}
	// not found
	return NULL;
}

void CGuiView::OnDrawMenuItem(LPDRAWITEMSTRUCT param)
{
	CMenu *menuSub=FindPopupMenuFromID(&_menu,param->itemID);

}

void CGuiView::OnMeasureMenuItem(LPMEASUREITEMSTRUCT param)
{
	ASSERT_VALID(&_menu);
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	// start from popup
	CMenu *pMenu = CMenu::FromHandle(pThreadState->m_hTrackingMenu);
//	pMenu = CMenu::FromHandle(_menu.m_hMenu);
	ASSERT_VALID(pMenu);

	int c=_menu.GetMenuItemCount();
	CMenu *menuSub=FindPopupMenuFromID(&_menu,param->itemID);

}

void CGuiView::_ScreenToGG(int &x,int &y)
{
	x=(int)((((float)x)-_offGG.x)/_scaleGG.x);
	if (!_bYInverse)
		y=(int)((((float)y)-_offGG.y)/_scaleGG.y);
	else
		y=(int)((((float)y)-_offGG.y)/(-_scaleGG.y));
}


//////////////////////////////////////////////////////////////////////////
//CGuiActor
void CGuiActor::DoCommand(DWORD idCmd)
{
	switch(idCmd)
	{
	case GuiCmd_Undo:
		{
			if (_modmgr)
				_modmgr->Undo();
			break;
		}
	case GuiCmd_Redo:
		{
			if (_modmgr)
				_modmgr->Redo();
			break;
		}
	}
}

void CGuiActor::UpdateCommandUI(DWORD idCmd,void *param)
{
	CCmdUI *pCmdUI=(CCmdUI *)param;
	switch(idCmd)
	{
		case GuiCmd_Undo:
		{
			if (_modmgr)
			{
				if (_modmgr->CanUndo())
					pCmdUI->Enable(TRUE);
				else
					pCmdUI->Enable(FALSE);
			}
			else
				pCmdUI->Enable(FALSE);
			break;
		}
		case GuiCmd_Redo:
		{
			if (_modmgr)
			{
				if (_modmgr->CanRedo())
					pCmdUI->Enable(TRUE);
				else
					pCmdUI->Enable(FALSE);
			}
			else
				pCmdUI->Enable(FALSE);
			break;
		}
	}

}



//////////////////////////////////////////////////////////////////////////
//CGuiPanel

BEGIN_MESSAGE_MAP(CGuiPanel, CDialog)
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()


CGuiPanel::CGuiPanel(UINT id,CWnd *pParent):CDialog(id,pParent)
{
}

BOOL CGuiPanel::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		BOOL bEditFocus=FALSE;
		CWnd *pWnd=GetFocus();
		if (pWnd)
		{
			TCHAR szTemp[32];
			::GetClassName(pWnd->GetSafeHwnd(), szTemp, ARRAY_SIZE(szTemp));
			if (strcmp(toMBCS(szTemp), "Edit") == 0)
				bEditFocus=TRUE;
		}
		if (!bEditFocus)
			return FALSE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}


//////////////////////////////////////////////////////////////////////////
//CGuiMgr
CGeActor *CGuiMgr::GetActiveActor()
{
	extern BOOL CheckWndDescendant(CWnd *pWnd,CWnd *pWndToCheck);
	CWnd *wnd=CWnd::GetFocus();

	//find the focus actor
	for (int i=0;i<_views.size();i++)
	{
		CWnd *wndTest=((CGuiView*)_views[i])->GetWnd();
		if (wndTest)
		{
			if ((wnd==wndTest)||(CheckWndDescendant(wndTest,wnd)))
				return _views[i]->GetCurActor();
		}
	}

	for (int i=0;i<_actors.size();i++)
	{
		CWnd *wndTest=((CGuiActor*)_actors[i])->GetWnd();
		if (wndTest)
		{
			if ((wnd==wndTest)||(CheckWndDescendant(wndTest,wnd)))
				return _actors[i];
		}
	}

	return NULL;
}

void CGuiMgr::Update()
{

	CGeMgr::Update();


	//send timer message to each view
	CPoint pt;
	GetCursorPos(&pt);
	for (int i=0;i<_views.size();i++)
	{
		CGuiView *view=(CGuiView *)_views[i];
		LRESULT ret;
		view->RespondMsg(pt,WM_TIMER,0,0,ret);
	}
}

void CGuiMgr::DoCommand(DWORD idCmd)
{
	if (_modmgrs.size()==1)
	{
		CModManager *modmgr=(*_modmgrs.begin()).second;
		switch(idCmd)
		{
			case GuiCmd_Undo:
			{
				modmgr->Undo();
				return;
			}
			case GuiCmd_Redo:
			{
				modmgr->Redo();
				return;
			}
		}
	}

	CGeMgr::DoCommand(idCmd);
}


void CGuiMgr::UpdateCommandUI(DWORD idCmd,void *param)
{
	CCmdUI *pCmdUI=(CCmdUI *)param;

	if (_modmgrs.size()==1)
	{
		CModManager *modmgr=(*_modmgrs.begin()).second;
		switch(idCmd)
		{
			case GuiCmd_Undo:
			{
				pCmdUI->Enable(modmgr->CanUndo());
				return;
			}
			case GuiCmd_Redo:
			{
				pCmdUI->Enable(modmgr->CanRedo());
				return;
			}
		}
	}

	if(!GetActiveActor())
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	CGeMgr::UpdateCommandUI(idCmd,param);
}

