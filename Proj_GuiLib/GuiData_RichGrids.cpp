#include "stdh.h"

#include "GuiData_RichGrids.h"

#include "WndBase.h"


const char *GuiData_RichGrids::GetCurGrid()
{
	std::unordered_map<std::string,CRichGrid*>::iterator it;
	for (it=_grids.begin();it!=_grids.end();it++)
	{
		CWnd*wnd=(CWnd*)(*it).second;
		if (!wnd)
			continue;
		if (!wnd->IsWindowVisible())
			continue;

		CWnd *wndFocus=CWnd::GetFocus();
		if ((wndFocus==wnd)||CheckWndDescendant(wnd,wndFocus))
			return (*it).first.c_str();
	}

	return "";
}
