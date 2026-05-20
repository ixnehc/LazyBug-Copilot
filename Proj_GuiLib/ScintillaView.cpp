#include "stdh.h"
#include "scintilla.h"

#include "ScintillaView.h"




// CScintillaView

IMPLEMENT_DYNCREATE(CScintillaView, CView)

BEGIN_MESSAGE_MAP(CScintillaView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_NOTIFY(SCN_MODIFIED,ID_WNDSCINTILLA,OnModify)
	ON_NOTIFY(SCN_CHARADDED,ID_WNDSCINTILLA,OnCharAdded)
	ON_NOTIFY(SCN_MODIFYATTEMPTRO, ID_WNDSCINTILLA, OnModifyAttemptRO)
	ON_NOTIFY(SCN_DWELLSTART,ID_WNDSCINTILLA,OnDwellStart)
	ON_NOTIFY(SCN_DWELLEND,ID_WNDSCINTILLA,OnDwellEnd)
	ON_NOTIFY(SCN_DOUBLECLICK,ID_WNDSCINTILLA,OnDblClick)
	ON_NOTIFY(SCN_UPDATEUI,ID_WNDSCINTILLA,OnUpdateUI)

	ON_WM_ERASEBKGND()
//	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
//	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
//	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
//	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
//	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
//	ON_COMMAND(ID_EDIT_FINDNEXT, OnEditFindnext)
END_MESSAGE_MAP()


CScintillaView::CScintillaView()
{
	_wnd=NULL;
	_bModified=FALSE;
	_bSettingText = FALSE;
}

CScintillaView::~CScintillaView()
{
}

BOOL CScintillaView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CScintillaView drawing

void CScintillaView::OnDraw(CDC* /*pDC*/)
{

	// TODO: add draw code for native data here
}


// CScintillaView message handlers

int CScintillaView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	_wnd=_NewWnd();
	if (!_wnd)
		return -1;

	if (!_wnd->Create(_T("Title"), WS_CHILD | WS_VISIBLE, CRect(0,0,1,1), this, ID_WNDSCINTILLA)) // hb - todo autogenerate id
		return -1;

	_SetDefaultFormat();

	return 0;
}

void CScintillaView::OnDestroy()
{
	_DeleteWnd(_wnd);

	CView::OnDestroy();
}


void CScintillaView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	if (_wnd->GetSafeHwnd())
	{
		_wnd->MoveWindow(0, 0, cx, cy);
	}

}

void CScintillaView::SetText(const char *text)
{
	_bSettingText = TRUE;
	_wnd->SetText(text);
	_bSettingText = FALSE;

	_wnd->EmptyUndoBuffer();

	ClearModified();
}


void CScintillaView::OnEditRedo()
{
	// TODO: Add your command handler code here
	_wnd->Redo();
}

void CScintillaView::OnEditUndo()
{
	// TODO: Add your command handler code here
	_wnd->Undo();
}

BOOL CScintillaView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return 0;
	return CView::OnEraseBkgnd(pDC);
}


void CScintillaView::OnModify(NMHDR* pNMHDR, LRESULT* pResult)
{
	SCNotification*scn= (SCNotification*)pNMHDR;

	if (scn->modificationType&(SC_MOD_INSERTTEXT|SC_MOD_DELETETEXT))
	{
		_bModified=TRUE;
		if (!_bSettingText)
			_OnModified();
	}



	*pResult=1;
}

void CScintillaView::OnCharAdded(NMHDR*pNMHDR, LRESULT*pResult)
{
	SCNotification*scn= (SCNotification*)pNMHDR;

	_OnCharAdded(scn->ch);

	*pResult=1;
}

void CScintillaView::OnModifyAttemptRO(NMHDR* pNMHDR, LRESULT* pResult)
{
	SCNotification* scn = (SCNotification*)pNMHDR;

	_OnModifyAttemptRO();

	*pResult = 0;
}


void CScintillaView::OnDwellStart(NMHDR*pNMHDR, LRESULT*pResult)
{
	SCNotification*scn= (SCNotification*)pNMHDR;
	_OnDwellStart((int)scn->position);
	*pResult=1;
}

void CScintillaView::OnDwellEnd(NMHDR*pNMHDR, LRESULT*pResult)
{
	SCNotification*scn= (SCNotification*)pNMHDR;
	_OnDwellEnd((int)scn->position);
	*pResult=1;
}

void CScintillaView::OnDblClick(NMHDR*pNMHDR, LRESULT*pResult)
{
	SCNotification*scn= (SCNotification*)pNMHDR;
	_OnDblClick();
	*pResult=1;
}

void CScintillaView::OnUpdateUI(NMHDR*pNMHDR, LRESULT*pResult)
{
	_OnUpdateUI();
	*pResult=1;
}


const char *CScintillaView::GetText()
{
	return _wnd->GetText();
}

void CScintillaView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// TODO: Add your specialized code here and/or call the base class
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
	if (bActivate)
		_wnd->SetFocus();
}

void CScintillaView::OnActivateFrame(UINT nState, CFrameWnd* pDeactivateFrame)
{
	// TODO: Add your specialized code here and/or call the base class

	CView::OnActivateFrame(nState, pDeactivateFrame);
}


void CScintillaView::OnEditCut()
{
	_wnd->Cut();
}
void CScintillaView::OnEditCopy()
{
	_wnd->Copy();

}
void CScintillaView::OnEditPaste()
{
	_wnd->Paste();
}

void CScintillaView::OnEditFindnext()
{
	// TODO: Add your command handler code here
	_wnd->SearchForwardAuto();
}

void CScintillaView::GotoLine(int iLine)
{
	_wnd->GotoLine(iLine);
	_wnd->SetFocus();
}


void CScintillaView::_SetDefaultFormat()
{
// 	_wnd->SetBackground(STYLE_DEFAULT,0);
// 	_wnd->SetForeground(STYLE_DEFAULT,0xffffffff);
// 	_wnd->SetBackground(STYLE_LINENUMBER,0);
// 
// 
// 	_wnd->SetLexer(SCLEX_LUA);
// 
// 	int size=16;
// 	const char *face="微软雅黑";
// 	_wnd->SetStyle(SCE_LUA_DEFAULT, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_IDENTIFIER, 0xffffff,0x000000,size,face);	
// 
// 	_wnd->SetStyle(SCE_LUA_COMMENT, 0xafafaf,0x000000,size,face);
// 
// 	_wnd->SetStyle(SCE_LUA_COMMENTLINE, 0xafafaf,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_COMMENTDOC, 0xafafaf,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_NUMBER, 0x00ff00,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_WORD, 0x00ffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_STRING, 0x0000ff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_CHARACTER, 0x0000ff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_PREPROCESSOR, 0x00FFFF,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_OPERATOR, 0xffff00,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_IDENTIFIER, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_STRINGEOL, 0x0000ff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_WORD2, 0x7f7fff,0,size,face);
// 	_wnd->SetStyle(SCE_LUA_WORD3, 0xffc000,0,size,face);
// 
// 	_wnd->SetCaretFore(0xffffff);
// 
// 	_wnd->SetSelColor(0x0,0xafafaf);
// 
// 	_wnd->SetMarginWidth(32);
// 
// 	_wnd->SetTipFore(0);

}
