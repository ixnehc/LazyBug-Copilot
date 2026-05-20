
#pragma once
#include "GuiLib.h"

#include <map>

class CSlidePanel;

class GuiLib_Api CSlideTab:public CXTPTabControl
{
public:
	virtual void Reposition();

	CXTPTabManagerItem* InsertItem(int nItem, LPCTSTR lpszItem,CSlidePanel *panel, int nImage= -1);

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	std::map<HWND,CSlidePanel *>_mapPanels;


};