/********************************************************************
	created:	2008/02/26
	created:	26:2:2008   9:38
	filename: 	e:\IxEngine\Proj_GuiLib\ScrollWnd.cpp
	file path:	e:\IxEngine\Proj_GuiLib
	file base:	ScrollWnd
	file ext:	cpp
	author:		star
	purpose:	a scrool window container ,supports scrooling function.
*********************************************************************/

#include "stdh.h"
#include "ScrollWnd.h"

BEGIN_MESSAGE_MAP(CScroolWnd,CWnd)

END_MESSAGE_MAP()

BOOL CScroolWnd::AddControl(CWnd * pControl)
{
	if(!pControl)
		return FALSE;
	
	CRect  rc;
	i_math::vector2du  wh;
	
	pControl->GetWindowRect(&rc);
	wh.x=rc.Height();
	wh.y=rc.Width();
    
	_controls[pControl]=wh;
	return TRUE;
}

BOOL CScroolWnd::SetControlRect(CWnd * pControl ,DWORD height,DWORD width)
{
	control_iterator it=_controls.find(pControl);
	if(it==_controls.end()) 
		return FALSE;
	
	i_math::vector2du * pWh =&((*it).second);	
	pWh->x=height;
	pWh->y=width;

	return TRUE;
}

BOOL CScroolWnd::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
		case WM_MOUSEMOVE:
			{
				AfxMessageBox("wm_move");
				break;
			}
	}
	return TRUE;
}

BOOL CScroolWnd::Create(RECT & rc,CWnd *pParent,UINT templateId,DWORD wsStyle)
{
	BOOL ret = CWnd::Create(NULL,NULL,wsStyle,rc,pParent,templateId);	
	if(!ret) 
		return FALSE;
	
	EnableScrollBar(SB_VERT);
	EnableScrollBar(SB_HORZ);
	
	return TRUE;
}

BOOL CScroolWnd::OnPaint(CDC* pDC)
{
		
	return TRUE;
}





