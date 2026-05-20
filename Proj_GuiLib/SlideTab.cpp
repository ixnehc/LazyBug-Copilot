/********************************************************************
	created:	2006/9/4   18:14
	filename: 	e:\IxEngine\Proj_GuiLib\SlideTab.cpp
	author:		cxi
	
	purpose:	a tab could slide its client area
*********************************************************************/
#include "stdh.h"
#include ".\SlideTab.h"
#include "SlidePanel.h"

CXTPTabManagerItem* CSlideTab::InsertItem(int nItem, LPCTSTR lpszItem,
										  CSlidePanel *panel, int nImage)
{
	CXTPTabManagerItem *pItem;
	pItem=CXTPTabControl::InsertItem(nItem,lpszItem,panel->GetSafeHwnd(),nImage);
	if (!pItem)
		return NULL;

	_mapPanels[panel->GetSafeHwnd()]=panel;
	return pItem;
}

void CSlideTab::Reposition()
{
	if (!GetSafeHwnd())
		return;

	CXTPClientRect rc(this);
	CClientDC dc(this);

	GetPaintManager()->RepositionTabControl(this, &dc, rc);
	GetPaintManager()->AdjustClientRect(this, rc);

	HWND hwndClient=NULL;
	if (m_pSelected)
		hwndClient=m_pSelected->GetHandle();

	if (hwndClient && ::IsWindow(hwndClient))
	{
		CSlidePanel *panel=NULL;
		std::map<HWND,CSlidePanel *>::iterator it;
		it=_mapPanels.find(hwndClient);
		if (it!=_mapPanels.end())
			panel=(*it).second;
		if (!panel)
			::MoveWindow(hwndClient, rc.left, rc.top, rc.Width(), rc.Height(), TRUE);
		else
		{
			if (panel->_ptOff.x+panel->_sz.cx<rc.right)
				panel->_ptOff.x=rc.right-panel->_sz.cx;
			if (panel->_ptOff.x>rc.left)
				panel->_ptOff.x=rc.left;
			if (panel->_ptOff.y<rc.top)
				panel->_ptOff.y=rc.top;
			::MoveWindow(hwndClient, panel->_ptOff.x, panel->_ptOff.y, 
				rc.right-panel->_ptOff.x, rc.bottom-panel->_ptOff.y, TRUE);
		}
	}

	Invalidate(FALSE);

}



BOOL CSlideTab::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message==WM_MOUSEWHEEL)
		return FALSE;

	return CXTPTabControl::PreTranslateMessage(pMsg);

}
