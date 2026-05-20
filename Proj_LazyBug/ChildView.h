#pragma once

#include "GuiViewWnd.h"
#include "CppView.h"

#include "gds/GObj.h"
#include "WEditor_MainFrame.h"

#include "codediff/CodeDiff.h"
#include "FileChange.h"

#include "CppSymbol.h"

#include "Utils.h"

#define MARKER_BREAKPOINT 2
#define MARKER_EXE_INDICATOR 3
#define MARKER_EXE_INDICATOR2 4
#define MARKER_DIFF_OLD_LINE 5
#define MARKER_DIFF_OLD_LINE_STRIKETHROUGH 6
#define MARKER_DIFF_NEW_LINE 7
#define MARKER_REPARING_LINE 8


class CChildDoc;
class CWEditor_ChildFrame;

class CScintillaWnd;
#define ID_TIMER_UPDATE 1001  // 定时器ID

struct LspSemanticTokens;

struct CodeComparingOldChars;
class CChildView : public CScintillaView
{
protected: // create from serialization only
	CChildView();
	DECLARE_DYNCREATE(CChildView)

// Attributes
public:
	CChildDoc* GetDocument()	{		return (CChildDoc*)CView::GetDocument();	}

	CView * AddView(CRuntimeClass* pViewClass, LPCTSTR lpszTitle, int nIcon,BOOL bCloseBtn=FALSE);
	void UpdateDocTitle();

	void ReloadContent();

	void AttachChange(const FileChange& change);
	void DetachChange();

	void Update();

	void UpdateTab();
	void UpdateSyntaxAssist();
	void UpdateExeIndicator();
	void UpdateUI();

	void CloseTabItem(CXTPTabManagerItem* item, BOOL bSaveModified);

	CWEditor_ChildFrame *GetEditor()	{		return _editor;	}

	const char* GetContent()	{		return _content.content.c_str();	}
	const char* GetLowerCasedFilePath()	{		return _content.lowerCasedPath.c_str();	}

	void InvalidateAllView();

	BOOL IsModified();

	void Colorize(const std::deque< CppSymbol::CSymbolDefine> &refs);
	void Colorize(const LspSemanticTokens &tokens);

	void UpdateDocumentTitle();

	void UpdateSaveModified();

	void UpdateGotoDefinition();
	void UpdateSelectRequest();

	void RequestSelect(LspRange range)
	{
		_selectRequest = range;
	}

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

	void _CullVar(int pos, std::string& s);
	void _CullPrevVar(int pos, std::string& s);

	void _OnCharAdded(int ch) override;
	void _OnModifyAttemptRO() override;
	void _OnModified()override;
	void _OnDwellStart(int pos)override;
	void _OnDwellEnd(int pos)override;
	void _OnUpdateUI()override;

	void _MakeAutoRepair(char c);

	void _SetDefaultFormat()override;
	std::string _keywords;

	struct FileContent
	{
		FileContent()
		{
			fileTime = 0;
			modifiedTime = 0;
			fileChangeTime = 0;
			requestId = CppSymbol::ParseRequestId_Invalid;
			codingFmt = Utils::FileContentCodingFormat::None;
		}
		bool IsValid()
		{
			return fileTime != 0;
		}
		bool IsInRequest()
		{
			return requestId != CppSymbol::ParseRequestId_Invalid;
		}

		std::string lowerCasedPath;

		std::string content;//当前编辑器的内容
		Utils::FileContentCodingFormat codingFmt;
		AbsTick fileTime;
		AbsTick modifiedTime;

		FileChange fileChange;
		AbsTick fileChangeTime;
		CodeComparingLines comparingContent;

		CppSymbol::ParseRequestId requestId;
	};
	FileContent _content;

	void _RefreshText(std::string& newText);

public:
	virtual ~CChildView();

protected:
	CWEditor_ChildFrame *_editor;

// 	CAutoCompleteListCpp _aclist;

	LspRange _selectRequest;

	bool _isPendingDetachChange;

	const char* _SkipCodingHead(const char* content);

	void _FlushTriggerInstruct();


	void _EnsureVisible(int line);

// Generated message map functions
protected:
	//{{AFX_MSG(CChildView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg void OnSelectedChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNextDiff();
	afx_msg void OnPrevDiff();
	afx_msg void OnGotoDefine();
	afx_msg void OnFileSave();
	afx_msg void OnPostKeyDown(NMHDR*, LRESULT*);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnDestroy();

	friend class CMainFrame;

	// 跳转到差异块,根据传入的参数决定是向前还是向后寻找
	// bForward: TRUE表示向后寻找,FALSE表示向前寻找
	void GotoDiffBlockPosition(BOOL bForward);

	// 自动修补子函数
	void _AutoBracketMatch(char c, int curPos); // 括号匹配
	void _AutoBraceMatch(int curPos, int lineStartPos, int lineEndPos, const std::string& curLineText); // 花括号匹配与处理
	void _AutoQuoteMatch(char c, int curPos); // 引号匹配
	void _AutoIndent(int curLine, int lineStartPos); // 自动缩进
	void _AutoClassEnd(int curLine, int curPos, int lineEndPos); // 类结束处理
	void _ProcessIncludeDirective(char c, int curPos, int lineStartPos, const std::string& curLineText); // 处理include指令
	void _ProcessSemicolon(int curPos, int lineEndPos, const std::string& curLineText); // 处理分号
	void _AlignSwitchCase(int curLine, int curPos, int lineStartPos); // 处理switch-case对齐

	void _SetCursorPos(int pos);

	void _SetOldCharsHilight(CodeComparingOldChars& oldChars);
	void _ClearOldCharsHilight();


};
