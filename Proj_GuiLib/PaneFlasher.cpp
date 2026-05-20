#include "stdh.h"
#include "resource.h"
#include "WndBase.h"

#include "PaneFlasher.h"

#include "DockingPane/XTPDockingPaneDefines.h"
#include "DockingPane/XTPDockingPaneManager.h"
#include "DockingPane/XTPDockingPanePaintManager.h"
#include "DockingPane/XTPDockingPaneBase.h"
#include "DockingPane/XTPDockingPaneBaseContainer.h"
#include "DockingPane/XTPDockingPane.h"
#include "DockingPane/XTPDockingPaneTabbedContainer.h"

#include "log/LogFile.h"


//////////////////////////////////////////////////////////////////////////
//CPaneFlasher
void CPaneFlasher::Init(int id,CXTPDockingPaneManager *mgr,UINT idIcon)
{
	_id=id;
	_mgr=mgr;

	_hIcon[0].CreateIconFromResource(MAKEINTRESOURCE(IDI_LOGREPORTFLASH),CSize(16,16));
	_hIcon[1].CreateIconFromResource(MAKEINTRESOURCE(IDI_LOGREPORTFLASH1),CSize(16,16));
	_hIcon[2].CreateIconFromResource(MAKEINTRESOURCE(idIcon),CSize(16,16));

	_mgr->SetIcon(_id,_hIcon[2]);

}

void CPaneFlasher::Flash()
{
	if (!_bFlashing)
	{
		_tStart=GetTickCount();
		_bFlashing=TRUE;
	}
}

void CPaneFlasher::Update()
{
	CXTPDockingPane* pane=_mgr->FindPane(_id);
	if (!pane)
		return;
	if (_bFlashing)
	{
		if (pane->GetChild())
		{
			if (pane->GetChild()->IsWindowVisible())
				_bFlashing=FALSE;
		}
	}


	int stateCur;
	if (_bFlashing)
	{
		const DWORD cycle=500;
		DWORD t=GetTickCount();
		stateCur=((t-_tStart)/cycle)%2;
	}
	else
		stateCur=2;


	if (stateCur!=_state)
	{
		_mgr->SetIcon(_id,_hIcon[stateCur]);

		if (pane->IsHidden())
		{
			//我们没法直接得到auto hide的panel,只好出此下策
			HWND hParent,hChild;
			hParent=AfxGetMainWnd()->GetSafeHwnd();
			hChild=NULL;
			CWnd *wnd=CWnd::FindWindowEx(hParent,hChild,_T("XTPDockingPaneAutoHidePanel"), _T(""));
			while(wnd)
			{
				wnd->InvalidateRect(NULL);
				hChild=wnd->GetSafeHwnd();
				wnd=CWnd::FindWindowEx(hParent,hChild, _T("XTPDockingPaneAutoHidePanel"), _T(""));
			}
		}
		else
		{
			CXTPDockingPaneBase *container=pane->GetContainer();
			if (container)
			if (container->GetType()==xtpPaneTypeTabbedContainer)
			{
				((CWnd*)((CXTPDockingPaneTabbedContainer*)container))->InvalidateRect(NULL);
			}
		}

		_state=stateCur;
	}

}
