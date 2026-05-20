/********************************************************************
	created:	2008/4/22   13:50
	file path:	d:\IxEngine\Proj_WorldEditor2
	author:		cxi
	
	purpose:	a view to contain some tabbed view 
*********************************************************************/

#include "stdh.h"
#include "commondefines/general_stl.h"

#include "ChildDoc.h"
#include "ChildView.h"

#include "MainFrm.h"

#include "ScintillaWnd.h"

#include "scintilla.h"
#include "SciLexer.h"

#include "WEditor_childframe.h"

#include "Utils.h"
#include "SolutionDBAPI.h"

#include "SymbolRefsCache.h"
#include "CodingHistory.h"


#define SCE_C_NAMESPACE 28

#define SCE_CUSTOMIZE_START 32
#define SCE_C_TYPEDEF (SCE_CUSTOMIZE_START+0)
#define SCE_C_MEMBER (SCE_CUSTOMIZE_START+2)
#define SCE_C_VARIABLE (SCE_CUSTOMIZE_START+3)
#define SCE_C_FUNCTION (SCE_CUSTOMIZE_START+4)
#define SCE_C_CLASS (SCE_CUSTOMIZE_START+5)
#define SCE_C_ENUM (SCE_CUSTOMIZE_START+6)


extern CWEditor_MainFrame* GetEditorMainFrame();
extern CMainFrame* GetMainFrame();

extern CSmartRepair g_smartRepair;


/////////////////////////////////////////////////////////////////////////////
// CChildView

IMPLEMENT_DYNCREATE(CChildView, CView)

BEGIN_MESSAGE_MAP(CChildView, CScintillaView)
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
//	ON_NOTIFY(TCN_SELCHANGE, IDC_TABCONTROL, OnSelectedChanged)
	ON_WM_MOUSEACTIVATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_COMMAND(ID_NEXT_DIFF, OnNextDiff)
	ON_COMMAND(ID_PREV_DIFF, OnPrevDiff)
	ON_COMMAND(ID_GOTO_DEFINE, OnGotoDefine)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_NOTIFY(SCN_CUSTOM_POST_KEYDOWN, ID_WNDSCINTILLA, OnPostKeyDown)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildView construction/destruction

CChildView::CChildView()
{
	_editor = NULL;
	_isPendingDetachChange = false;
}

CChildView::~CChildView()
{
}

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if (!CScintillaView::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	return TRUE;

}

/////////////////////////////////////////////////////////////////////////////
// CChildView drawing

void CChildView::OnDraw(CDC*)
{
}


CView *CChildView::AddView(CRuntimeClass* pViewClass, LPCTSTR lpszTitle, int nIcon,BOOL bCloseBtn)
{
// 	CCreateContext contextT;
// 	contextT.m_pCurrentDoc     = GetDocument();
// 	contextT.m_pNewViewClass   = pViewClass;
// 	contextT.m_pNewDocTemplate = GetDocument()->GetDocTemplate();
// 
// 	CWnd* pWnd;
// 	TRY
// 	{
// 		pWnd = (CWnd*)pViewClass->CreateObject();
// 		if (pWnd == NULL)
// 		{
// 			AfxThrowMemoryException();
// 		}
// 	}
// 	CATCH_ALL(e)
// 	{
// 		TRACE0( "Out of memory creating a view.\n" );
// 		// Note: DELETE_EXCEPTION(e) not required
// 		return NULL;
// 	}
// 	END_CATCH_ALL
// 
// 	DWORD dwStyle = AFX_WS_DEFAULT_VIEW;
// 	dwStyle &= ~WS_BORDER;
// 
// 	int nTab = _wndTabCtrl.GetItemCount();
// 
// 	// Create with the right size (wrong position)
// 	CRect rect(0,0,0,0);
// 	if (!pWnd->Create(NULL, NULL, dwStyle,
// 		rect, &_wndTabCtrl, (AFX_IDW_PANE_FIRST + nTab), &contextT))
// 	{
// 		TRACE0( "Warning: couldn't create client tab for view.\n" );
// 		// pWnd will be cleaned up by PostNcDestroy
// 		return NULL;
// 	}
// 	CXTPTabManagerItem *item=_wndTabCtrl.InsertItem(nTab, lpszTitle, pWnd->GetSafeHwnd(), nIcon);
// 	CXTPTabManagerNavigateButtons *btns=item->GetNavigateButtons();
// 
// 	for (int i=0;i<btns->GetSize();i++)
// 	{
// 		CXTPTabManagerNavigateButton *btn=btns->GetAt(i);
// 		if (bCloseBtn)
// 			btn->SetFlags(xtpTabNavigateButtonAlways);
// 		else
// 			btn->SetFlags(xtpTabNavigateButtonNone);
// 	}
// 
// 	item->SetData((DWORD_PTR)pWnd);
// 
// 	pWnd->SetOwner(this);
// 
// 	return (CView*)pWnd;

	return NULL;
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScintillaView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// 设置定时器,每100ms触发一次
	SetTimer(ID_TIMER_UPDATE, 100, NULL);

	return 0;
}


void CChildView::OnInitialUpdate()
{
	CScintillaView::OnInitialUpdate();

	CChildDoc* pDoc = GetDocument();
	if (!pDoc)
		return;

	// 获取文件路径
	CString strPath = pDoc->GetPathName();
	if (strPath.IsEmpty())
		return;

	// 读取文件内容
	CT2A asciiPath(strPath);

	if (!Utils::GetFileContentIntoUTF8(asciiPath.m_psz, _content.content,_content.codingFmt))
		return;

	_content.lowerCasedPath = asciiPath.m_psz;
	StringLower(_content.lowerCasedPath);

	// 获取文件修改时间
	_content.fileTime = Utils::GetFileTick(asciiPath.m_psz);
	_content.modifiedTime = _content.fileTime;

	// 设置到Scintilla窗口
	SetText(Utils::SkipCodingHead(_content.content.c_str()));

	// 设置语法高亮
	_SetDefaultFormat();

// 	_aclist.Create(_wnd, 4022);
// 	_aclist.BindToEdit(_wnd);
// 	_aclist.SetFilePath(_content.path);

}

void CChildView::OnDestroy()
{
	CScintillaView::OnDestroy();

	// 清理定时器
	KillTimer(ID_TIMER_UPDATE);

// 	extern CMainFrame *g_pProtoLibWnd;
 // 
// 	g_pProtoLibWnd->DestroyProtoEditor(_editor,this);

}

void CChildView::ReloadContent()
{
	CChildDoc* pDoc = GetDocument();
	if (!pDoc)
		return;

	// 获取文件路径
	CString strPath = pDoc->GetPathName();
	if (strPath.IsEmpty())
		return;

	// 读取文件内容
	CT2A asciiPath(strPath);

	AbsTick fileTime = Utils::GetFileTick(asciiPath.m_psz);
	if (fileTime == _content.fileTime)
		return;

	std::string newContent;
	Utils::FileContentCodingFormat codingFmt;
	if (!Utils::GetFileContentIntoUTF8(asciiPath.m_psz, newContent,codingFmt))
		return;

	if (newContent == _content.content)
		return;

	_content.content = std::move(newContent);
	_content.codingFmt = codingFmt;

	if (_content.fileChange.IsEmpty())
		_RefreshText(_content.content);

	// 获取文件修改时间
	_content.fileTime = fileTime;
	_content.modifiedTime = _content.fileTime;

}



BOOL CChildView::OnEraseBkgnd(CDC*)
{
	return TRUE;
}

void CChildView::OnPaint()
{
	CPaintDC dc(this);
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CScintillaView::OnSize(nType, cx, cy);

}

void CChildView::UpdateDocTitle()
{
	GetDocument()->UpdateFrameCounts();
}

void CChildView::AttachChange(const FileChange& change)
{
	_content.fileChange = change;
	_content.fileChange.newContent.push_back(0);
	_content.fileChange.oldContent.push_back(0);
	_content.fileChangeTime = GetAbsTick();

	_wnd->SetReadOnly(FALSE);

	std::string oldContent = Utils::SkipCodingHead((const char*)&_content.fileChange.oldContent[0]);
	std::string newContent = Utils::SkipCodingHead((const char*)&_content.fileChange.newContent[0]);

	_content.comparingContent.Clear();
	MakeCodeComparing_Lines(oldContent, newContent, _content.comparingContent);

	_RefreshText(_content.comparingContent.content);

	for (int i = 0;i < _content.comparingContent.lineOrigins.size();i++)
	{
		auto tp = _content.comparingContent.lineOrigins[i].tp;
		if (tp == CodeComparingLines::LineOrigin::NewCode)
			_wnd->AddBookmark(i, MARKER_DIFF_NEW_LINE);
		if (tp == CodeComparingLines::LineOrigin::OldCode)
		{
			_wnd->AddBookmark(i, MARKER_DIFF_OLD_LINE);
			_wnd->AddBookmark(i, MARKER_DIFF_OLD_LINE_STRIKETHROUGH);
		}
	}

	_wnd->SetReadOnly(TRUE);

	_wnd->SetCurrentLine(0);
	OnNextDiff();
}

void CChildView::DetachChange()
{
	if (_content.fileChange.IsEmpty())
		return;
	_content.fileChange.Clear();

	_wnd->SetReadOnly(FALSE);

	_RefreshText(_content.content);
}



void CChildView::OnSelectedChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
// 	UNUSED_ALWAYS(pNMHDR);
// 	*pResult = 0;
// 
// 	UpdateDocTitle();
// 
// 	CFrameWnd* pFrame = GetParentFrame();
// 	CView* pView = DYNAMIC_DOWNCAST(CView, CWnd::FromHandle(_wndTabCtrl.GetSelectedItem()->GetHandle()));
// 	ASSERT_KINDOF(CView, pView);
// 
// 	pFrame->SetActiveView(pView);
}


int CChildView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}


void CChildView::Update()
{
}


void CChildView::UpdateTab()
{
}

void CChildView::UpdateSyntaxAssist()
{
// 	CXTPTabManagerItem*item=_wndTabCtrl.GetSelectedItem();
// 	CProtoLuaSource *p=_ProtoLuaSrcFromItem(item);
// 	if (!p)
// 		return;
// 
// 	if (!_editor)
// 		return;
// 
// 	p->UpdateSyntaxAssist(_editor->_dataProto.proto());
}

void CChildView::UpdateUI()
{
// 	CXTPTabManagerItem*item=_wndTabCtrl.GetSelectedItem();
// 	CProtoLuaSource *p=_ProtoLuaSrcFromItem(item);
// 	if (!p)
// 		return;
// 
// 	if (!_editor)
// 		return;
// 
// 	p->UpdateUI();
}

void CChildView::InvalidateAllView()
{
}

void SetScintillaDefaultFormat(CScintillaWnd* wnd)
{
	if (!wnd)
		return;

	// C++关键字
	const char keywords[] =
		"and and_eq asm auto bitand bitor bool break case catch char char16_t char32_t class compl "
		"const constexpr const_cast continue decltype default delete do double dynamic_cast else enum "
		"explicit export extern false float for friend goto if inline int long mutable namespace new "
		"noexcept not not_eq nullptr operator or or_eq private protected public register reinterpret_cast "
		"return short signed sizeof static static_assert static_cast struct switch template this thread_local "
		"throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while "
		"xor xor_eq include pragma once __int64";

	// 设置C++语法高亮
	wnd->SetKeywords(0, keywords);
	wnd->SetLexer(SCLEX_CPP);

	wnd->SetCodePage(SC_CP_UTF8);

	// 设置样式
	wnd->SetBackground(STYLE_DEFAULT, 0);
	wnd->SetForeground(STYLE_DEFAULT, 0xffffffff);
	wnd->SetBackground(STYLE_LINENUMBER, 0);

	int size = 15;
	const char* face = "Comic Sans MS";

	wnd->SetStyle(SCE_C_DEFAULT, 0xffffff, 0x000000, size, face);
	wnd->SetStyle(SCE_C_COMMENT, 0xafafaf, 0x000000, size, face);
	wnd->SetStyle(SCE_C_COMMENTLINE, 0xafafaf, 0x000000, size, face);
	wnd->SetStyle(SCE_C_COMMENTDOC, 0xafafaf, 0x000000, size, face);
	wnd->SetStyle(SCE_C_NUMBER, 0x00ff00, 0x000000, size, face);
	wnd->SetStyle(SCE_C_WORD, 0x00ffff, 0x000000, size, face);
	wnd->SetStyle(SCE_C_STRING, 0x0000ff, 0x000000, size, face);
	wnd->SetStyle(SCE_C_CHARACTER, 0x0000ff, 0x000000, size, face);
	wnd->SetStyle(SCE_C_PREPROCESSOR, 0x0000ff, 0x000000, size, face);
	wnd->SetStyle(SCE_C_OPERATOR, 0xffff00, 0x000000, size, face);
	wnd->SetStyle(SCE_C_IDENTIFIER, 0xffffff, 0x000000, size, face);
	wnd->SetStyle(SCE_C_GLOBALCLASS, 0x007fff, 0x000000, size, face);
	wnd->SetStyle(SCE_C_USERLITERAL, 0xDBCDBF, 0x000000, size, face);
	wnd->SetStyle(SCE_C_NAMESPACE, 0xDBCDBF, 0x000000, size, face);
	// 	wnd->SetStyle(SCE_C_TYPEDEF, 0xff7f00, 0x000000, size, face);
	// 	wnd->SetStyle(SCE_C_MEMBER, 0xffffff, 0x000000, size, face);
	// 	wnd->SetStyle(SCE_C_VARIABLE, 0xffffff, 0x000000, size, face);
	// 	wnd->SetStyle(SCE_C_FUNCTION, 0xffffff, 0x000000, size, face);
	// 	wnd->SetStyle(SCE_C_ENUM, 0x9f9fff, 0x000000, size, face);

	wnd->SetCaretFore(0xffffff);

	wnd->SetSelColor(0x0, 0xafafaf);

	wnd->SetMarginWidth(32);

	wnd->SetTipFore(0);

	wnd->DefineBookmark(MARKER_DIFF_OLD_LINE, 0, 0x00003f, SC_MARK_BACKGROUND);
	wnd->DefineBookmark(MARKER_DIFF_NEW_LINE, 0, 0x003f00, SC_MARK_BACKGROUND);
	wnd->DefineBookmark(MARKER_DIFF_OLD_LINE_STRIKETHROUGH, 0xffffff, 0x000000, '-');
	wnd->DefineBookmark(MARKER_REPARING_LINE, 0, 0x003f3f, SC_MARK_BACKGROUND);
}

void CChildView::_SetDefaultFormat()
{
	CScintillaView::_SetDefaultFormat();
	SetScintillaDefaultFormat(_wnd);
}

void CChildView::_OnCharAdded(int ch)
{
	CWEditor_MainFrame* editor = GetEditorMainFrame();
	if (!editor)
		return;

	g_smartRepair.GetTrigger().OnCharTyped((char)ch);
	_FlushTriggerInstruct();
}


void CChildView::_OnModified()
{
	CWEditor_MainFrame* editor = GetEditorMainFrame();
	if (!editor)
		return;

	if (!_content.fileChange.IsEmpty())
		return;
	std::string newContent= GetText();
	if (newContent == _content.content)
		return;

	if (true)
	{
		extern CCodingHistory g_codingHistory;
		if (_content.lowerCasedPath != g_codingHistory.GetCurEditFilePath())
			g_codingHistory.SetCurEditFile(_content.lowerCasedPath.c_str(), _content.content);
		g_codingHistory.Edit(_content.lowerCasedPath.c_str(), newContent);

		std::string str;
		g_codingHistory.Dump(str, 4096);

		std::string dumpPath= GetModuleFolderPath(NULL);
		dumpPath += "\\coding_history.txt";

		Utils::SaveFileContent(dumpPath.c_str(), str);
	}

	_content.content = std::move(newContent);
	_content.modifiedTime = GetAbsTick();

	extern CSymbolRefsCache g_symbolRefsCache;
	g_symbolRefsCache.SetDirty(_content.lowerCasedPath);

	if (!g_symbolRefsCache.IsCaching())
	{
		g_symbolRefsCache.CacheRefs(_content.lowerCasedPath, _content.content);
	}


}

void CChildView::_OnModifyAttemptRO()
{
	if (!_content.fileChange.IsEmpty())
	{
		_isPendingDetachChange = true;
	}
}


inline BOOL IsWordChar(int c)
{
	if (c <= 0)
		return FALSE;
	return isalnum(c) || (c == '_');
}


void CChildView::_CullVar(int pos0, std::string& s)
{
	int pos = pos0;
	int c;

	//往后找
	while (IsWordChar(c = _wnd->GetCharAt(pos++)))
		s += c;

	if (s == "")
		return;

	//往前找Word|dot|blank
	pos = pos0 - 1;
	while (1)
	{
		c = _wnd->GetCharAt(pos--);

		if (IsWordChar(c) || (c == ' ') || (c == '.'))
			s = std::string("") + (char)c + s;
		else
			break;
	}

	std::vector<std::string>buf, buf2;
	std::list<std::string>buf3;

	//用dot分割
	SplitStringBy(".", s, &buf);

	std::string ss;
	//从后到前遍历
	for (int i = buf.size() - 1;i >= 0;i--)
	{
		ss = buf[i];
		//分割后的每个部分切去首尾
		RemoveTailBlank(ss);
		RemoveHeadBlank(ss);
		if (ss == "")
			break;

		//用空格分开
		SplitStringBy(" ", ss, &buf2);

		//分割后的最后一块添加到结果字串的前部
		buf3.push_front(buf2[buf2.size() - 1]);

		//如果分割出多块,则不用继续了
		if (buf2.size() > 1)
			break;
	}

	LinkStringBy(".", s, &buf3);

}

void CChildView::_CullPrevVar(int pos, std::string& s)
{
	int c;
	while (1)
	{
		c = _wnd->GetCharAt(pos--);

		if (IsWordChar(c))
			s = std::string("") + (char)c + s;
		else
			break;
	}

}

void CChildView::_OnDwellStart(int pos)
{
	std::string s;
	_CullVar(pos, s);

}

void CChildView::_OnDwellEnd(int pos)
{
	_wnd->HideTip();
}


void CChildView::_OnUpdateUI()
{
}


BOOL CChildView::IsModified()
{
	if (_content.modifiedTime > _content.fileTime)
		return TRUE;
	return FALSE;
}


void CChildView::UpdateSaveModified()
{
	AbsTick curTime = GetAbsTick();

	CWEditor_MainFrame* editor = GetEditorMainFrame();
	if (!editor)
		return;
	if (curTime > _content.modifiedTime + 100)
	{
		if (_content.IsValid())
		{
			if (_content.modifiedTime > _content.fileTime)
			{
				if (Utils::SetFileContentFromUTF8(_content.lowerCasedPath.c_str(), _content.content,_content.codingFmt))
				{
					_content.fileTime = Utils::GetFileTick(_content.lowerCasedPath.c_str());
					_content.modifiedTime = _content.fileTime;
				}
			}
		}
	}
}

void CChildView::UpdateDocumentTitle()
{
	if (!_content.IsValid())
		return;
	std::string name = GetFileName(_content.lowerCasedPath);
	if (IsModified())
		name += "*";
	if (GetDocument())
		GetDocument()->SetTitle(CString(name.c_str()));
}


void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	CScintillaView::OnTimer(nIDEvent);

	UpdateDocumentTitle();

//	UpdateSaveModified();

	UpdateGotoDefinition();
	UpdateSelectRequest();

	if (_isPendingDetachChange)
	{
		DetachChange();
		_isPendingDetachChange = false;
	}

	if (!_content.lowerCasedPath.empty())
	{
		extern CSymbolRefsCache g_symbolRefsCache;
		if (g_symbolRefsCache.CheckDirty(_content.lowerCasedPath))
		{
			if (!g_symbolRefsCache.IsCaching())
				g_symbolRefsCache.CacheRefs(_content.lowerCasedPath, _content.content);
		}
	}

	if (GetMainFrame())
	{
		if (GetMainFrame()->GetActiveView() == this)
		{
			g_smartRepair.Update();
			_FlushTriggerInstruct();
		}
	}



//	UpdateCompletionList();
}

void CChildView::OnNextDiff()
{
	GotoDiffBlockPosition(TRUE); // 向后寻找
}

void CChildView::OnPrevDiff()
{
	GotoDiffBlockPosition(FALSE); // 向前寻找
}

void CChildView::GotoDiffBlockPosition(BOOL bForward)
{
	if (_content.fileChange.IsEmpty())
		return;

	CScintillaWnd* pWnd = GetScintillaWnd();
	if (!pWnd)
		return;

	// 获取当前行号
	int currentLine = pWnd->GetCurrentPosition();
	currentLine = pWnd->LineFromPosition(currentLine);
	
	// 确定当前行的差异类型(如果当前行是差异行)
	int currentLineDiffType = -1;
	if (pWnd->HasBookmark(currentLine, MARKER_DIFF_NEW_LINE))
		currentLineDiffType = MARKER_DIFF_NEW_LINE;
	else if (pWnd->HasBookmark(currentLine, MARKER_DIFF_OLD_LINE))
		currentLineDiffType = MARKER_DIFF_OLD_LINE;
	
	// 首次查找的起始位置
	int searchStartLine = bForward ? currentLine + 1 : currentLine - 1;
	
	// 查找起始行有效性检查
	int lineCount = pWnd->GetLineCount();
	if (searchStartLine < 0)
		searchStartLine = 0;
	if (searchStartLine >= lineCount)
		searchStartLine = lineCount - 1;
	
	int targetLine = -1;
	int searchLine = searchStartLine;
	bool foundNonDiffLine = false;
	
	// 查找下一个/上一个差异行
	if (bForward)
		targetLine = pWnd->FindNextBookmark((1 << MARKER_DIFF_OLD_LINE) | (1 << MARKER_DIFF_NEW_LINE) | (1 << MARKER_DIFF_OLD_LINE_STRIKETHROUGH), searchLine, FALSE);
	else
		targetLine = pWnd->FindPreviousBookmark((1 << MARKER_DIFF_OLD_LINE) | (1 << MARKER_DIFF_NEW_LINE) | (1 << MARKER_DIFF_OLD_LINE_STRIKETHROUGH), searchLine, FALSE);
	
	// 如果没有找到差异行,则不进行循环查找,直接结束
	if (targetLine != -1)
	{
		// 确定找到的差异行类型
		int foundDiffType = -1;
		if (pWnd->HasBookmark(targetLine, MARKER_DIFF_NEW_LINE))
			foundDiffType = MARKER_DIFF_NEW_LINE;
		else if (pWnd->HasBookmark(targetLine, MARKER_DIFF_OLD_LINE))
			foundDiffType = MARKER_DIFF_OLD_LINE;
		
		// 检查当前行和找到的差异行之间是否存在无差异行
		if (currentLineDiffType != -1)
		{
			foundNonDiffLine = false;
			int startCheck = currentLine < targetLine ? currentLine + 1 : targetLine + 1;
			int endCheck = currentLine < targetLine ? targetLine - 1 : currentLine - 1;
			
			for (int i = startCheck; i <= endCheck; i++)
			{
				if (!pWnd->HasBookmark(i, MARKER_DIFF_NEW_LINE) && !pWnd->HasBookmark(i, MARKER_DIFF_OLD_LINE))
				{
					foundNonDiffLine = true;
					break;
				}
			}
			
			// 如果未找到无差异行且不是不同类型的差异,继续搜索
			if (!foundNonDiffLine && currentLineDiffType == foundDiffType)
			{
				// 继续查找下一个差异行
				searchLine = bForward ? targetLine + 1 : targetLine - 1;
				
				// 检查搜索行范围,如果超出范围则结束搜索
				if (searchLine < 0 || searchLine >= lineCount)
				{
					// 已到达文件边界,不进行循环
					targetLine = -1;
				}
				else
				{
					// 继续在有效范围内搜索
					int newTargetLine = -1;
					if (bForward)
						newTargetLine = pWnd->FindNextBookmark((1 << MARKER_DIFF_OLD_LINE) | (1 << MARKER_DIFF_NEW_LINE) | (1 << MARKER_DIFF_OLD_LINE_STRIKETHROUGH), searchLine, FALSE);
					else
						newTargetLine = pWnd->FindPreviousBookmark((1 << MARKER_DIFF_OLD_LINE) | (1 << MARKER_DIFF_NEW_LINE) | (1 << MARKER_DIFF_OLD_LINE_STRIKETHROUGH), searchLine, FALSE);
					
					if (newTargetLine == -1)
					{
						// 没有找到更多差异行
						targetLine = -1;
					}
					else
					{
						// 检查新找到的差异行类型
						int newFoundDiffType = -1;
						if (pWnd->HasBookmark(newTargetLine, MARKER_DIFF_NEW_LINE))
							newFoundDiffType = MARKER_DIFF_NEW_LINE;
						else if (pWnd->HasBookmark(newTargetLine, MARKER_DIFF_OLD_LINE))
							newFoundDiffType = MARKER_DIFF_OLD_LINE;
						
						// 如果还是相同类型且没有非差异行,继续搜索
						if (currentLineDiffType == newFoundDiffType)
						{
							foundNonDiffLine = false;
							startCheck = currentLine < newTargetLine ? currentLine + 1 : newTargetLine + 1;
							endCheck = currentLine < newTargetLine ? newTargetLine - 1 : currentLine - 1;
							
							for (int i = startCheck; i <= endCheck; i++)
							{
								if (!pWnd->HasBookmark(i, MARKER_DIFF_NEW_LINE) && !pWnd->HasBookmark(i, MARKER_DIFF_OLD_LINE))
								{
									foundNonDiffLine = true;
									break;
								}
							}
							
							if (foundNonDiffLine || currentLineDiffType != newFoundDiffType)
							{
								// 找到了非差异行或不同类型差异行,使用这个目标
								targetLine = newTargetLine;
							}
							else
							{
								// 仍然没有非差异行且是相同类型,不使用这个目标
								targetLine = -1;
							}
						}
						else
						{
							// 找到了不同类型的差异行,使用这个目标
							targetLine = newTargetLine;
						}
					}
				}
			}
		}
	}
	
	// 如果找到了差异行,则跳转到该行
	if (targetLine != -1)
	{
		// 首先跳转到目标行
		pWnd->GotoLine(targetLine);
		
		// 计算能够显示的总行数
		int linesVisible = (int)pWnd->SendMessage(SCI_LINESONSCREEN, 0, 0);
		
		// 尝试将当前行居中显示在编辑器中
		if (linesVisible > 0)
		{
			// 获取编辑器的第一个可见行
			int firstVisibleLine = (int)pWnd->SendMessage(SCI_GETFIRSTVISIBLELINE, 0, 0);
			
			// 计算需要滚动的行数,使目标行居中
			int scrollLines = targetLine - (firstVisibleLine + linesVisible / 2);
			
			// 如果需要滚动,则调整视图
			if (scrollLines != 0)
			{
				// 向下滚动(正值)或向上滚动(负值)
				pWnd->SendMessage(SCI_LINESCROLL, 0, scrollLines);
			}
		}
		
//		pWnd->SelectLine();
	}
}

void CChildView::_EnsureVisible(int targetLine)
{
	CScintillaWnd* pWnd = GetScintillaWnd();
	if (!pWnd)
		return;

	// 首先跳转到目标行
	pWnd->GotoLine(targetLine);

	// 计算能够显示的总行数
	int linesVisible = (int)pWnd->SendMessage(SCI_LINESONSCREEN, 0, 0);

	// 尝试将当前行居中显示在编辑器中
	if (linesVisible > 0)
	{
		// 获取编辑器的第一个可见行
		int firstVisibleLine = (int)pWnd->SendMessage(SCI_GETFIRSTVISIBLELINE, 0, 0);

		// 计算需要滚动的行数,使目标行居中
		int scrollLines = targetLine - (firstVisibleLine + linesVisible / 2);

		// 如果需要滚动,则调整视图
		if (scrollLines != 0)
		{
			// 向下滚动(正值)或向上滚动(负值)
			pWnd->SendMessage(SCI_LINESCROLL, 0, scrollLines);
		}
	}
}

void CChildView::OnFileSave()
{
	UpdateSaveModified();
}


void CChildView::OnGotoDefine()
{
}

void CChildView::UpdateGotoDefinition()
{
}

void CChildView::UpdateSelectRequest()
{
	if (!_selectRequest.IsEmpty())
	{
		_wnd->SetSelection(_selectRequest.start.line, _selectRequest.start.character, _selectRequest.end.line, _selectRequest.end.character);
		_EnsureVisible(_selectRequest.start.line);
	}
	_selectRequest.Reset();
}


void CChildView::_RefreshText(std::string& newText)
{
	int curLine = _wnd->GetCurrentLine();
	int curVisibleLine = _wnd->GetFirstVisibleLine();

	std::deque<CodeDiffLine> diffs;

	std::string oldText = _wnd->GetText();
	CompareCodeStrings(oldText, newText, diffs);

	int newLine;
	int newVisibleLine;

	newLine= ClosestNewLineFromOldLine(curLine, diffs);
	newVisibleLine= ClosestNewLineFromOldLine(curVisibleLine, diffs);

	SetText(newText.c_str());

	_wnd->SetCurrentLine(newLine);
	_wnd->SetFirstVisibleLine(newVisibleLine);

}

void CChildView::_SetCursorPos(int pos)
{
	if (_wnd)
	{
		_wnd->SetCurrentPosition(pos);
		_wnd->SetSelection(pos, pos);
	}
}


// 括号自动匹配
void CChildView::_AutoBracketMatch(char c, int curPos)
{
    CScintillaWnd* pWnd = GetScintillaWnd();
    if (!pWnd)
        return;

    // 对于括号字符，自动添加对应的右括号
    char rightBracket = (c == '(') ? ')' : ']';
    pWnd->InsertText(&rightBracket, curPos);
}


// 大括号自动匹配和处理
void CChildView::_AutoBraceMatch(int curPos, int lineStartPos, int lineEndPos, const std::string& curLineText)
{
    CScintillaWnd* pWnd = GetScintillaWnd();
    if (!pWnd)
        return;

    // 检查是否是在行末
    if (curPos >= lineEndPos - 1)
    {
        // 计算当前行的缩进
        int indentCount = 0;
		for (size_t i = 0; i < curLineText.length(); i++)
		{
			if (curLineText[i] == ' ' || curLineText[i] == '\t')
			{
				if (curLineText[i] == '\t')
					indentCount++;
			}
			else
				break;
		}
        
        std::string indent(indentCount, '\t');
        
        // 插入换行和缩进
        pWnd->InsertText(("\n" + indent + "\t" + "\n" + indent + "}").c_str(), curPos);
        
        // 将光标移动到中间行
        _SetCursorPos(curPos + 2 + indent.length());
    }
    else
    {
        // 如果不是在行末，只插入对应的闭合大括号
        pWnd->InsertText("}", curPos);
    }
}

// 自动处理类结束
void CChildView::_AutoClassEnd(int curLine, int curPos, int lineEndPos)
{
    CScintillaWnd* pWnd = GetScintillaWnd();
    if (!pWnd)
        return;

    // 检查是类的结束大括号，自动添加分号
    if (curPos >= lineEndPos - 1) // 如果在行末
    {
        // 向上查找类定义
        bool isClassEnd = false;
        for (int i = curLine - 1; i >= 0 && i >= curLine - 100; i--) // 向上最多查找100行
        {
            std::string lineText = pWnd->GetLineText(i);
            
            // 检测是否有类定义
            if (lineText.find("class ") != std::string::npos || 
                lineText.find("struct ") != std::string::npos)
            {
                isClassEnd = true;
                break;
            }
            
            // 如果遇到另一个结束大括号，则退出查找
            if (lineText.find("}") != std::string::npos)
                break;
        }
        
        if (isClassEnd)
        {
            pWnd->InsertText(";", curPos);
        }
    }
}

// 引号自动匹配
void CChildView::_AutoQuoteMatch(char c, int curPos)
{
    CScintillaWnd* pWnd = GetScintillaWnd();
    if (!pWnd)
        return;

    // 自动添加闭合引号
	std::string s;
	s += c;
    pWnd->InsertText(s.c_str(), curPos);
}

// 处理预处理指令
void CChildView::_ProcessIncludeDirective(char c, int curPos, int lineStartPos, const std::string& curLineText)
{
    CScintillaWnd* pWnd = GetScintillaWnd();
    if (!pWnd)
        return;

    if (c == '#')
    {
        // 如果是行首的#，并且整行为空，则可能是要输入#include
        if (curPos == lineStartPos)
        {
            // 自动补全#include
            pWnd->InsertText("include ", curPos);
            pWnd->SetCurrentPosition(curPos + 8); // 移动光标到include后
        }
    }
    else if (c == '<')
    {
        // 检查是否在#include行
        std::string trimmedLine = curLineText;
        RemoveHeadBlank(trimmedLine);
        if (trimmedLine.find("#include") == 0)
        {
            // 在#include行自动匹配尖括号
            pWnd->InsertText(">", curPos);
        }
    }
    else if (c == '>')
    {
        // 检查下一个字符是否已经是>，如果是则跳过
        if (pWnd->GetCharAt(curPos) == '>')
        {
            // 如果已经有>，则移动光标但不插入
            pWnd->SetCurrentPosition(curPos + 1);
        }
    }
}

// 处理分号
void CChildView::_ProcessSemicolon(int curPos, int lineEndPos, const std::string& curLineText)
{
    CScintillaWnd* pWnd = GetScintillaWnd();
    if (!pWnd)
        return;

    // 检查是否在行末添加分号
    if (curPos >= lineEndPos - 1)
    {
        // 如果是for循环内的分号，不添加换行
        if (curLineText.find("for") != std::string::npos && 
            curLineText.find("(") != std::string::npos)
        {
            return;
        }

        // 在一些语句后自动换行并保持缩进
        int indentCount = 0;
        for (size_t i = 0; i < curLineText.length(); i++)
        {
            if (curLineText[i] == ' ' || curLineText[i] == '\t')
                indentCount++;
            else
                break;
        }
        
        // 添加换行和缩进
        if (indentCount > 0)
        {
            std::string indent(indentCount, ' ');
            pWnd->InsertText(std::string("\n" + indent).c_str(), curPos);
            pWnd->SetCurrentPosition(curPos + 1 + indent.length());
        }
    }
}

// 自动缩进
void CChildView::_AutoIndent(int curLine, int lineStartPos)
{
    CScintillaWnd* pWnd = GetScintillaWnd();
    if (!pWnd)
        return;
        
    // 获取上一行的文本
    std::string prevLineText = pWnd->GetLineText(curLine - 1);
    
    // 计算上一行的缩进
    int indentCount = 0;
    for (size_t i = 0; i < prevLineText.length(); i++)
    {
		if (prevLineText[i] == ' ' || prevLineText[i] == '\t')
		{
			if (prevLineText[i] == '\t')
				indentCount++;
		}
        else
            break;
    }
    
    std::string indent(indentCount, '\t');
    
    // 检查上一行是否需要额外缩进（以关键字结尾）
    std::string trimmedPrevLine = prevLineText;
    RemoveHeadBlank(trimmedPrevLine);
    RemoveTailBlank(trimmedPrevLine);
    
    if (trimmedPrevLine.length() > 0)
    {
        // 检查是否以这些关键字结尾
//         const char* keywords[] = {"if", "for", "while", "else", "switch", "case", "do"};
//         for (const char* keyword : keywords)
//         {
//             // 检查是否是完整的关键字（不是变量名的一部分）
//             size_t pos = trimmedPrevLine.find(keyword);
//             if (pos != std::string::npos)
//             {
//                 size_t keywordLen = strlen(keyword);
//                 // 确保关键字前后是空格或特殊字符
//                 bool validBefore = (pos == 0 || !IsWordChar(trimmedPrevLine[pos-1]));
//                 bool validAfter = (pos + keywordLen == trimmedPrevLine.length() || 
//                                   !IsWordChar(trimmedPrevLine[pos + keywordLen]));
//                 
//                 if (validBefore && validAfter)
//                 {
//                     needExtraIndent = true;
//                     break;
//                 }
//             }
//         }
        
        // 检查是否以左括号或冒号结尾
        if (trimmedPrevLine.back() == '{' || trimmedPrevLine.back() == ':')
			indent += '\t';
    }
    
    // 如果需要额外缩进
    if (true)
    {
        // 检查是否是if/for/while语句，并且没有{，则自动添加大括号
        bool autoAddBraces = false;
		bool isClass=false;
        for (const char* keyword : {"if", "for", "while","class","struct","enum"})
        {
            if (trimmedPrevLine.find(keyword) == 0 && 
                trimmedPrevLine.find("{") == std::string::npos &&
                trimmedPrevLine.back() != ':')
            {
                autoAddBraces = true;
				if ((keyword == "class") || (keyword == "struct")|| (keyword == "enum"))
					isClass = true;
                break;
            }
        }
        
        if (autoAddBraces)
        {
            // 在当前行前插入{
            pWnd->InsertText((indent+"{\n" ).c_str(), lineStartPos);
			int nextLinePos = pWnd->PositionFromLine(curLine + 1);
			pWnd->InsertText((indent + "\t\n").c_str(), nextLinePos);
			if (!isClass)
				pWnd->InsertText((indent + "}").c_str(), pWnd->PositionFromLine(curLine + 2));
			else
				pWnd->InsertText((indent + "};").c_str(), pWnd->PositionFromLine(curLine + 2));

//             // 计算下一行的缩进
//             std::string nextIndent(indentCount, '\t');
//             
//             // 在当前行后插入\n}
//             int newEndPos = pWnd->EndPosFromLine(curLine);
//             pWnd->InsertText(("\n" + nextIndent + "}").c_str(), newEndPos);
            
            // 调整光标位置
            _SetCursorPos(nextLinePos + 1 + indent.length());
            return;
        }
    }
    
    // 插入适当的缩进
    if (indent.length() > 0)
    {
        pWnd->InsertText(indent.c_str(), lineStartPos);
        // 调整光标位置
		_SetCursorPos(lineStartPos + indent.length());
    }
}

// 处理switch-case自动对齐
void CChildView::_AlignSwitchCase(int curLine, int curPos, int lineStartPos)
{
    CScintillaWnd* pWnd = GetScintillaWnd();
    if (!pWnd)
        return;
        
    // 检查光标前的5个字符是否是"case "
    if (curPos >= lineStartPos + 5)
    {
        std::string lastFiveChars;
        for (int i = curPos - 5; i < curPos; i++)
        {
            lastFiveChars += (char)pWnd->GetCharAt(i);
        }
        
        if (lastFiveChars == "case ")
        {
            // 寻找上一个case语句的位置进行对齐
            for (int i = curLine - 1; i >= 0; i--)
            {
                std::string lineText = pWnd->GetLineText(i);
                if (lineText.find("case ") != std::string::npos || lineText.find("default:") != std::string::npos)
                {
                    // 获取case后的冒号位置
                    size_t colonPos = lineText.find(':');
                    if (colonPos != std::string::npos)
                    {
                        // 计算当前行到冒号的距离
                        int currentIndent = curPos - lineStartPos;
                        int targetIndent = colonPos + 1;
                        
                        if (targetIndent > currentIndent)
                        {
                            // 插入空格对齐冒号
                            std::string spaces(targetIndent - currentIndent, ' ');
                            pWnd->InsertText(spaces.c_str(), curPos);
                            pWnd->SetCurrentPosition(curPos + spaces.length());
                        }
                    }
                    break;
                }
            }
        }
    }
}

void CChildView::_MakeAutoRepair(char c)
{
	return;
	// 获取 ScintillaWnd 指针
	CScintillaWnd* pWnd = GetScintillaWnd();
	if (!pWnd)
		return;

	// 获取当前行和位置信息
	int curLine = pWnd->GetCurrentLine();
	int curPos = pWnd->GetCurrentPosition();
	int lineStartPos = pWnd->PositionFromLine(curLine);
	int lineEndPos = pWnd->EndPosFromLine(curLine);

	// 获取当前行文本
	std::string curLineText = pWnd->GetLineText(curLine);

	// 根据输入的字符进行自动修补处理
	switch (c)
	{
		// 括号自动匹配
	case '(':
	case '[':
		_AutoBracketMatch(c, curPos);
		break;

		// 大括号自动匹配和处理
	case '{':
		_AutoBraceMatch(curPos, lineStartPos, lineEndPos, curLineText);
		break;

		// 自动处理类结束
	case '}':
		_AutoClassEnd(curLine, curPos, lineEndPos);
		break;

		// 引号自动匹配
	case '"':
	case '\'':
		_AutoQuoteMatch(c, curPos);
		break;

		// 处理预处理指令
	case '#':
	case '<':
	case '>':
		_ProcessIncludeDirective(c, curPos, lineStartPos, curLineText);
		break;

		// 处理分号
	case ';':
//		_ProcessSemicolon(curPos, lineEndPos, curLineText);
		break;

		// 回车自动缩进
	case '\r':
		_AutoIndent(curLine, lineStartPos);
		break;

		// 处理switch-case自动对齐
	case 'e': // 可能是case关键字的结尾
		_AlignSwitchCase(curLine, curPos, lineStartPos);
		break;
	}
}

void CChildView::OnPostKeyDown(NMHDR* pNMHDR, LRESULT* pResult)
{
	SCNotification* scn = (SCNotification*)pNMHDR;

	g_smartRepair.GetTrigger().OnKeyDown(scn->ch);
	_FlushTriggerInstruct();

	*pResult = 1;
}

void CChildView::OnSetFocus(CWnd* pOldWnd)
{

	CScintillaView::OnSetFocus(pOldWnd);
}

void CChildView::OnKillFocus(CWnd* pNewWnd)
{
	CScintillaView::OnKillFocus(pNewWnd);
}

extern void AddLineSuffixForRepairing(const std::string& fileContent, int startLine, int endLine, std::string& fileContentWithLineNumber);

//得到CScintillaWnd窗口内容里当前cursor之前的内容和之后的内容
void SplitScintillaWndContentAtCursor(CScintillaWnd* wnd, std::string& prefix, std::string& suffix, std::string& sel)
{
	if (!wnd)
		return;

	// 获取整个文本内容
	std::string fullContent;
	wnd->GetText(fullContent);

	// 获取选中区域的起始和结束位置
	long selStart = wnd->GetSelectionStart();
	long selEnd = wnd->GetSelectionEnd();

	// 检查是否有选中区域
	if (selStart != selEnd && selStart >= 0 && selEnd >= 0)
	{
		// 确保选中区域在有效范围内
		if (selStart > fullContent.length()) selStart = fullContent.length();
		if (selEnd > fullContent.length()) selEnd = fullContent.length();

		// 获取选中的文本
		if (selStart < selEnd)
			sel = fullContent.substr(selStart, selEnd - selStart);
		else
			sel = fullContent.substr(selEnd, selStart - selEnd);

		// 选中区域之前的部分作为prefix
		prefix = fullContent.substr(0, selStart);

		// 选中区域之后的部分作为suffix
		suffix = fullContent.substr(selEnd);
	}
	else
	{
		// 没有选中区域，清空sel
		sel.clear();

		// 获取当前光标位置
		long cursorPos = wnd->GetCurrentPosition();
		if (cursorPos < 0)
			return;

		// 确保光标位置在有效范围内
		if (cursorPos > fullContent.length())
			cursorPos = fullContent.length();

		// 分割内容为前缀和后缀
		prefix = fullContent.substr(0, cursorPos);
		suffix = fullContent.substr(cursorPos);
	}
}

void CChildView::_FlushTriggerInstruct()
{
	CSmartRepairTrigger::Instruction instruct = g_smartRepair.GetTrigger().Poll();
	switch (instruct.action)
	{
		case CSmartRepairTrigger::Instruction::Action::PrepareContext:
		{
			CScintillaWnd* wnd = GetScintillaWnd();

			RepairRequest request;
			request.filePath = _content.lowerCasedPath;
			request.fileName = GetFileName(_content.lowerCasedPath);
			wnd->GetText(request.fileContent);
			SplitLines(request.fileContent, request.fileLines);
			request.cursorLine = wnd->GetCurrentLine();
			int cursorPos = wnd->GetCurrentPosition();
			int lineStartPos = wnd->PositionFromLine(request.cursorLine);
			request.cursorLinePrefix = request.fileLines[request.cursorLine].substr(0, cursorPos - lineStartPos);
			request.cursorLineSuffix = request.fileLines[request.cursorLine].c_str() + (cursorPos - lineStartPos);
			request.sessionId = instruct.sessionID;

			g_smartRepair.StartPrepareContext(request);
			break;
		}

		case CSmartRepairTrigger::Instruction::Action::TriggerAIRequest:
		{
			g_smartRepair.StartAIRequest();
			break;
		}
		case CSmartRepairTrigger::Instruction::Action::ShowSuggestion:
		{
			//XXXXX:RepairSolution

			g_smartRepair.ShowSuggestion(CRect(0, 0, 10, 10));
			break;
		}
		case CSmartRepairTrigger::Instruction::Action::HideSuggestion:
		{
			g_smartRepair.HideSuggestion();
			break;
		}
	}
}

void CChildView::_SetOldCharsHilight(CodeComparingOldChars& oldChars)
{
	CScintillaWnd* pWnd = GetScintillaWnd();
	if (!pWnd)
		return;

	// 清除之前的高亮
	_ClearOldCharsHilight();

	// 为每一行的指定字符设置高亮
	for (const auto& line : oldChars.lines)
	{
		if (line.line < 0 || line.line >= pWnd->GetLineCount())
			continue;

		// 获取该行的起始位置
		int lineStartPos = pWnd->PositionFromLine(line.line);
		int lineEndPos = pWnd->EndPosFromLine(line.line);
		
		// 为该行的每个指定字符位置设置高亮
		for (int charIndex : line.indices)
		{
			// 计算字符的绝对位置
			int charPos = lineStartPos + charIndex;
			
			// 确保位置有效
			if (charPos >= lineStartPos && charPos < lineEndPos)
			{
				// 设置单个字符的背景色（使用indicator）
				// 先设置indicator的样式（红色背景）
				pWnd->SendMessage(SCI_INDICSETSTYLE, 0, INDIC_ROUNDBOX);
				pWnd->SendMessage(SCI_INDICSETFORE, 0, RGB(255, 0, 0)); // 红色
				pWnd->SendMessage(SCI_INDICSETALPHA, 0, 100); // 半透明
				
				// 设置当前indicator
				pWnd->SendMessage(SCI_SETINDICATORCURRENT, 0, 0);
				
				// 为单个字符设置indicator
				pWnd->SendMessage(SCI_INDICATORFILLRANGE, charPos, 1);
			}
		}
	}
}

void CChildView::_ClearOldCharsHilight()
{
	CScintillaWnd* pWnd = GetScintillaWnd();
	if (!pWnd)
		return;

	// 清除所有indicator 0的高亮
	int textLength = pWnd->SendMessage(SCI_GETTEXTLENGTH, 0, 0);
	pWnd->SendMessage(SCI_SETINDICATORCURRENT, 0, 0);
	pWnd->SendMessage(SCI_INDICATORCLEARRANGE, 0, textLength);
}
