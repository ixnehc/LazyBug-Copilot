
#pragma once

#include "ScintillaWnd.h"

#include "ScintillaFind.h"

#define ID_WNDSCINTILLA 10000

class GuiLib_Api CScintillaView : public CView
{
public: // create from serialization only
	CScintillaView();
	DECLARE_DYNCREATE(CScintillaView)

// Attributes
public:
	CScintillaWnd *GetScintillaWnd()	{		return _wnd;	}
	void SetText(const char *text);
	const char *GetText();
	void ClearModified()	{		_bModified=FALSE;	}
	BOOL IsModified()	{		return _bModified;	}

	void GotoLine(int iLine);


// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CScintillaView();

protected:
	virtual void _SetDefaultFormat();
	virtual CScintillaWnd*_NewWnd()	{		return new CScintillaWnd;	}
	virtual void _DeleteWnd(CScintillaWnd*p)	{		delete p;	}
	virtual void _OnModified(){}
	virtual void _OnCharAdded(int ch)	{	}
	virtual void _OnModifyAttemptRO() {}
	virtual void _OnDblClick()	{	}
	virtual void _OnUpdateUI()	{	}


	virtual void _OnDwellStart(int pos)	{	}
	virtual void _OnDwellEnd(int pos)	{	}

	CScintillaWnd* _wnd;
	BOOL _bModified;

	BOOL _bSettingText;


// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEditRedo();
	afx_msg void OnEditUndo();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnModify(NMHDR*, LRESULT*);
	afx_msg void OnCharAdded(NMHDR*, LRESULT*);
	afx_msg void OnModifyAttemptRO(NMHDR*, LRESULT*);
	afx_msg void OnDwellStart(NMHDR*, LRESULT*);
	afx_msg void OnDwellEnd(NMHDR*, LRESULT*);
	afx_msg void OnDblClick(NMHDR*, LRESULT*);
	afx_msg void OnUpdateUI(NMHDR*, LRESULT*);
protected:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnActivateFrame(UINT nState, CFrameWnd* pDeactivateFrame);
public:
	afx_msg void OnEditFindnext();

};
