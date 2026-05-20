/********************************************************************
	created:	2006/08/01
	created:	1:8:2006   9:55
	filename: 	e:\IxEngine\Proj_GuiLib\WndBase.cpp
	file base:	WndBase
	file ext:	cpp
	author:		cxi
	
	purpose:	useful functions for general CWnd processing
*********************************************************************/
#include "stdh.h"

#include "TreeCtrlBase.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"


BOOL CreateImageList(CImageList& il, UINT nID,int w,int h)
{
	if (!il.Create(w,h, ILC_MASK|ILC_COLOR24, 1, 0))
		return FALSE;
	CBitmap bmp;
	VERIFY(bmp.LoadBitmap(nID));

	il.Add(&bmp, CXTPImageManager::GetBitmapMaskColor(bmp, CPoint(0, 0)));

	return TRUE;
}



GuiLib_Api BOOL GetWndClippedRect(CWnd *pWnd,CRect &rc,BOOL bConsiderChildren)
{
	CRgn rgn;
	CRect rcT;
	pWnd->GetClientRect(&rcT);
	rgn.CreateRectRgnIndirect(&rcT);
	CDC *pDC;
	if (bConsiderChildren)
		pDC=pWnd->GetDCEx(NULL,DCX_PARENTCLIP|DCX_CACHE|DCX_CLIPSIBLINGS|DCX_CLIPCHILDREN);
	else
		pDC=pWnd->GetDCEx(NULL,DCX_PARENTCLIP|DCX_CACHE|DCX_CLIPSIBLINGS);
//	CDC *pDC=pWnd->GetDC();
	if (!pDC)
		return FALSE;
	pDC->GetClipBox(&rc);
//	CPoint pt=pDC->GetViewportOrg();
//	CSize sz=pDC->GetViewportExt();
	pWnd->ReleaseDC(pDC);
	
	pWnd->ClientToScreen(&rc);
	return TRUE;
}


BOOL IS_BUTTON_CHECK(CWnd *pDlg,UINT id)
{
	CButton *pWnd=(CButton *)pDlg->GetDlgItem(id);
	if (pWnd)
		return (pWnd->GetCheck()!=0);
	return FALSE;
}


BOOL CheckWndDescendant(CWnd *pWnd,CWnd *pWndToCheck)
{
	if (pWndToCheck==pWnd)
		return FALSE;
	CWnd *wnd=pWndToCheck;
	while(wnd)
	{
		if (wnd==pWnd)
			return TRUE;
		wnd=wnd->GetParent();
	}
	return FALSE;
}

//得到顶级的父窗口
CWnd *GetMostAscendent(CWnd *pWnd)
{
	while(pWnd->GetParent())
		pWnd=pWnd->GetParent();
	return pWnd;
}


GuiLib_Api void SetWindowPos(CWnd *pWnd,i_math::recti &rc)
{
	if (pWnd->GetSafeHwnd())
		pWnd->SetWindowPos(NULL,rc.Left(),rc.Top(),rc.getWidth(),rc.getHeight(),SWP_NOZORDER);
}