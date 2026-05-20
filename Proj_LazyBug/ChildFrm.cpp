/********************************************************************
	created:	2008/4/22   13:58
	file path:	d:\IxEngine\Proj_WorldEditor2
	author:		cxi
	
	purpose:	child frame for protolib mdi window
*********************************************************************/

#include "stdh.h"

#include "WorldSystem/IEntitySystem.h"

#include "ChildFrm.h"
#include "ChildView.h"
#include "ChildDoc.h"

#include "WEditor_ChildFrame.h"

#include "stringparser/stringparser.h"


/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP

	ON_WM_MDIACTIVATE()
	ON_COMMAND(ID_TOOL_CHECKLUAINFO, OnToolCheckluainfo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here

}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

//	cs.style&=~WS_OVERLAPPEDWINDOW;
	cs.style|=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MAXIMIZEBOX|
							WS_MAXIMIZE;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;


	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers
void CChildFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
// 	// update our parent window first
// 	GetMDIFrame()->OnUpdateFrameTitle(bAddToTitle);
// 
// 	if ((GetStyle() & FWS_ADDTOTITLE) == 0)
// 		return;     // leave child window alone!
// 
// 	CDocument* pDocument = GetActiveDocument();
// 	if (bAddToTitle)
// 	{
// 		CString strWindowText;
// 		strWindowText = m_strTitle;
// 
// 		CChildView* pView = (CChildView*)GetDlgItem(AFX_IDW_PANE_FIRST);
// 		if (pView)
// 		{
// 			if (pView->IsModified())
// 				strWindowText += "*";
// 		}
// 	
// 		// set title if changed, but don't remove completely
// 		AfxSetWindowText(m_hWnd, strWindowText);
// 	}
}


void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

// 	CChildView* pView = (CChildView*)GetDlgItem(AFX_IDW_PANE_FIRST);
// 
// 	if (pView)
// 		pView->GetEditor()->Enable(bActivate);

}

void CChildFrame::OnToolCheckluainfo()
{
}
