#pragma once

/********************************************************************
	author:	yuyang
	date:	2009-07-03
	brief:	编辑brokenline的控件
*********************************************************************/

#include "GuiViewWnd.h"
#include "GuiView_ValueSet.h"
#include "GuiData_ValueSet.h"
#include "GuiActor_ValueSet.h"


class GuiLib_Api CValueSetEditor : public CGuiViewWnd<CWnd>
{
public:
	CValueSetEditor();
	~CValueSetEditor();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual BOOL Create( RECT &rc, CWnd *pParentWnd,UINT id);
};

