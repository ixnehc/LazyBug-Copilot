#pragma once

/********************************************************************
	author:	yuyang
	date:	2009-07-03
	brief:	编辑brokenline的控件
*********************************************************************/

#include "GuiViewWnd.h"
#include "GuiView_Changelists.h"
#include "GuiData_Changelists.h"
#include "GuiActor_Changelists.h"


class CChangelistsEditor : public CGuiViewWnd<CWnd>
{
public:
	CChangelistsEditor();
	~CChangelistsEditor();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual BOOL Create( RECT &rc, CWnd *pParentWnd,UINT id);
};

