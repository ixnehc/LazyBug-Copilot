#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>


#include "ShaderLibPanel.h"
#include "ShaderLibGlobal.h"


#include "TreeCtrlBase.h"

#pragma warning(disable:4018)


//////////////////////////////////////////////////////////////////////////
//CShaderLibPanel
BEGIN_MESSAGE_MAP(CShaderLibPanel, CWnd)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CShaderLibPanel::Create(CWnd *pParent,RECT &rc,UINT id)
{
	if (FALSE==CWnd::CreateEx(0,_T("STATIC"), NULL, WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, CRect(0, 0, 1, 1),pParent, 0))
		return FALSE;



	return TRUE;

}


int CShaderLibPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	_tree.Create(this,CRect(0,0,0,0),1000);

	_idTimer=(UINT)SetTimer((UINT_PTR)1,100,NULL);


	return 0;
}

	
void CShaderLibPanel::OnDestroy()
{
	KillTimer(_idTimer);
	CWnd::OnDestroy();
}


void CShaderLibPanel::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (_tree.GetSafeHwnd())
	{
		_tree.MoveWindow(0, 0, cx, cy );
		_tree.Invalidate(FALSE);
	}


}

void CShaderLibPanel::OnTimer(UINT_PTR idEvent)
{
	if (_global)
	{
		if (_ver!=_global->_ver)
		{
			_tree.Refresh();
			_ver=_global->_ver;
		}
	}
}

void CShaderLibPanel::SetContent(CShaderLibGlobal *global)
{
	_global=global;
	_tree.SetContent(global);

}
