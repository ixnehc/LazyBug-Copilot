/********************************************************************
	created:	2006/9/1   14:37
	filename: 	e:\IxEngine\Proj_GuiLib\ResEditPanel.cpp
	author:		ixnehc
	
	purpose:	tab ctrl to contain all the ResEditPanels
*********************************************************************/
#include "stdh.h"
#include ".\ResEditTab.h"


BOOL CResEditTab::Create(CWnd *pParent,CRect &rc,int id)
{
	if (FALSE==CXTPTabControl::Create(WS_VISIBLE|WS_CHILD,
		rc, pParent, id))
		return FALSE;

	GetPaintManager()->SetAppearance(xtpTabAppearancePropertyPage2003);
	GetPaintManager()->m_bHotTracking = TRUE;
	GetPaintManager()->m_bShowIcons = TRUE;
	GetPaintManager()->DisableLunaColors(FALSE);

	extern BOOL CreateImageList(CImageList& il, UINT nID,int w,int h);
	CreateImageList(_imglist,IDB_RESTREEICON,16,16);
	SetImageList(&_imglist);

	return TRUE;
}
