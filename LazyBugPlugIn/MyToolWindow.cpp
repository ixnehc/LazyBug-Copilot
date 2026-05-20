#include "stdh.h"
#include "guids.h"
#include "MyToolWindow.h"

#include <textfind.h> 
#include <chrono>

#include "PackageState.h"

#include "../Common/stringparser/stringparser.h"


extern void SaveSolutionDump(const char* fullPath, SolutionDump& slnDump);
extern void SaveSolutionDumpTimeStamp(const char* fullPath, SolutionDumpTimeStamps& timeStamp);
extern void LoadSolutionDumpTimeStamp(const char* fullPath, SolutionDumpTimeStamps& timeStamp);

bool g_requestGenerateSlnDump = false;
void RequestGenerateSlnDump()
{

	g_requestGenerateSlnDump = true;
}


STDMETHODIMP LazyBugPlugInWindowPane::TranslateAccelerator(LPMSG lpMsg)
{
	// 检查 m_hChatDialog 是否有效且是消息的目标之一
	// （更复杂的检查可能需要判断 lpMsg->hwnd 是否是 m_hChatDialog 或其子控件）
	if (m_hChatDialog && ::IsWindow(m_hChatDialog))
	{
		if (PreTranslateMessageToDialog(m_hChatDialog, *lpMsg))
			// 		if (::IsDialogMessage(m_hChatDialog, lpMsg))
		{
			return S_OK; // 消息已被处理
		}
	}
	return S_FALSE; // 消息未被处理，VS环境将继续处理它
}

void LazyBugPlugInWindowPane::PostSited(IVsPackageEnums::SetSiteResult result)
{
	VsWindowFrameEventSink<LazyBugPlugInWindowPane>::SetSite(GetVsSiteCache());

	CComPtr<IVsShell> spShell = GetVsSiteCache().GetCachedService<IVsShell, SID_SVsShell>();
	spShell->AdviseBroadcastMessages(this, &m_BroadcastCookie);
	InitVSColors();

}

void LazyBugPlugInWindowPane::PostClosed()
{
	// 停止定时器
	::KillTimer(this->GetHWND(), IDT_CHATDIALOG_TIMER);

	if (nullptr != m_hBackground)
	{
		::DeleteBrush(m_hBackground);
		m_hBackground = nullptr;
	}

	CComPtr<IVsShell> spShell = GetVsSiteCache().GetCachedService<IVsShell, SID_SVsShell>();
	if (nullptr != spShell && VSCOOKIE_NIL != m_BroadcastCookie)
	{
		spShell->UnadviseBroadcastMessages(m_BroadcastCookie);
		m_BroadcastCookie = VSCOOKIE_NIL;
	}

	// 取消注册Solution事件
	CComPtr<IVsSolution> spSolution = GetVsSiteCache().GetCachedService<IVsSolution, SID_SVsSolution>();
	if (spSolution && VSCOOKIE_NIL != m_SolutionCookie)
	{
		spSolution->UnadviseSolutionEvents(m_SolutionCookie);
		m_SolutionCookie = VSCOOKIE_NIL;
	}
}

void LazyBugPlugInWindowPane::OnFrameSize(int x, int y, int w, int h)
{
}

LRESULT LazyBugPlugInWindowPane::OnButtonClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	// 		// Load the message from the resources.
	// 		CComBSTR strMessage;
	// 		VSL_CHECKBOOL_GLE(strMessage.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_BUTTONCLICK_MESSAGE));
	// 
	// 		// Get the title of the message box (it is the same as the tool window's title).
	// 		CComBSTR strTitle;
	// 		VSL_CHECKBOOL_GLE(strTitle.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_WINDOW_TITLE));
	// 
	// 		// Get the UI Shell service.
	// 		CComPtr<IVsUIShell> spIVsUIShell = GetVsSiteCache().GetCachedService<IVsUIShell, SID_SVsUIShell>();
	// 		LONG lResult;
	// 		VSL_CHECKHRESULT(spIVsUIShell->ShowMessageBox(0, GUID_NULL, strTitle, strMessage, NULL, 0, OLEMSGBUTTON_OK, OLEMSGDEFBUTTON_FIRST, OLEMSGICON_INFO, FALSE, &lResult));

	bHandled = TRUE;
	return 0;
}

LRESULT LazyBugPlugInWindowPane::OnCtlColorDlg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (nullptr != m_hBackground)
	{
		bHandled = TRUE;
	}
	else
	{
		bHandled = FALSE;
	}

	return (LRESULT)m_hBackground;
}

LRESULT LazyBugPlugInWindowPane::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_hChatDialog)
	{
		::SetWindowPos(m_hChatDialog, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOACTIVATE);
	}
	bHandled = TRUE;
	return 0;
}


STDMETHODIMP LazyBugPlugInWindowPane::OnBroadcastMessage(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	switch (uMsg)
	{
	case WM_SYSCOLORCHANGE:
	case WM_PALETTECHANGED:
		// Re-initialize VS colors when the theme changes.
		InitVSColors();
		break;
	}

	return S_OK;
}

LRESULT LazyBugPlugInWindowPane::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 调用基类的 OnInitDialog (如果它有实现的话，通常 ATL/WTL 对话框基类会有)
	// 对于 VsWindowPaneFromResource，它内部处理了对话框的创建，
	// 我们在这里是对话框已经被创建之后。
	// bHandled = FALSE; // 如果希望基类也处理
	// return 1; // Typically return TRUE if you've set the focus

	// *** 在这里创建子窗口，因为此时 this->GetHWND() (父对话框) 是有效的 ***

	HWND hParentWnd = this->GetHWND(); // 现在这个句柄应该是有效的
	
	// 使用 OutputDebugString 代替 MessageBox 进行调试
	// OutputDebugStringW(L"MyToolWindow OnInitDialog: Creating Chat Dialog...\n");

	m_hChatDialog = CreateChatDialog(hParentWnd);

	if(m_hChatDialog)
	{
		// OutputDebugStringW(L"MyToolWindow OnInitDialog: Chat Dialog created successfully.\n");
	}

	::SetTimer(this->GetHWND(), IDT_CHATDIALOG_TIMER, 10, nullptr);

	bHandled = TRUE; // 我们处理了这个消息
	return FALSE; // 对于 WM_INITDIALOG，通常返回 TRUE (除非你把焦点设置到了某个控件，那种情况返回 FALSE)
}

void LazyBugPlugInWindowPane::InitVSColors()
{
	// Obtain IVsUIShell5 from IVsUIShell
	CComQIPtr<IVsUIShell5> spIVsUIShell5(GetVsSiteCache().GetCachedService<IVsUIShell, SID_SVsUIShell>());
	VS_RGBA vsColor;

	if (nullptr != m_hBackground)
	{
		::DeleteBrush(m_hBackground);
		m_hBackground = nullptr;
	}

	if (nullptr != spIVsUIShell5 && SUCCEEDED(spIVsUIShell5->GetThemedColor(EnvironmentColorsCategory, L"Window", TCT_Background, &vsColor)))
	{
		COLORREF crBackground = VS_RGBA_TO_COLORREF(vsColor);
		m_hBackground = ::CreateSolidBrush(crBackground);
	}

	if (::IsWindow(this->m_hWnd))
	{
		::InvalidateRect(this->m_hWnd, nullptr /* lpRect */, TRUE /* bErase */);
	}
}

void LazyBugPlugInWindowPane::EscapeChatInput()
{
	if (g_ps.pTextManager)
	{
		CComPtr<IVsTextView> pTextView;
		HRESULT hr = g_ps.pTextManager->GetActiveView(TRUE, NULL, &pTextView);
		if (SUCCEEDED(hr) && pTextView)
		{
			// 获取文本视图的窗口句柄
			HWND hwnd;
			hwnd = pTextView->GetWindowHandle();
			if (hwnd)
			{
				::SetFocus(hwnd);
			}
		}
	}
}


void LazyBugPlugInWindowPane::UpdateChatInputEscape()
{
	if (FetchChatInputEscapeRequest())
	{
		EscapeChatInput();
	}
}

void LazyBugPlugInWindowPane::UpdateSolutionDump()
{
	// 1. 检查是否打开了solution
	if (!g_ps.pSolution)
		return;

	// 获取solution文件路径
	const char* slnPathA = GetOpenedSlnPath_utf8();
	if (!slnPathA || slnPathA[0] == '\0')
		return;

	// 2. 获取solution db folder路径, 构建.slndmp和.slndmpts文件路径
	const char* dbFolderPath = GetOpenedDBFolderPath_utf8();
	if (!dbFolderPath || dbFolderPath[0] == '\0')
		return;

	std::string slnDmpPath = dbFolderPath;
	slnDmpPath += "\\.slndmp";

	if (!Util_ExistFile(slnDmpPath.c_str()))
	{
		if (m_GenerateSlnDumpProgress.IsEmpty())
			g_requestGenerateSlnDump = true;
	}

	if ((!g_requestGenerateSlnDump) && m_GenerateSlnDumpProgress.IsEmpty())
		return;

	if (m_GenerateSlnDumpProgress.IsEmpty())//进度中发生的请求,我们要保留
		g_requestGenerateSlnDump = false;//否则我们消耗掉这次请求

	bool success = Util_GenerateSolutionDump2(dbFolderPath, g_ps.pSolution, m_slnDmp,50, m_GenerateSlnDumpProgress);
	if (!success)
	{
		m_GenerateSlnDumpProgress.Zero();
		return;
	}

	if (m_GenerateSlnDumpProgress.IsDone())
	{
		m_GenerateSlnDumpProgress.Zero();
		SaveSolutionDump(slnDmpPath.c_str(), m_slnDmp);
	}

}

void LazyBugPlugInWindowPane::UpdateEventListener()
{
	bool isOpened = true;
	const char* slnPathA = GetOpenedSlnPath_utf8();
	if (!slnPathA || slnPathA[0] == '\0')
		isOpened = false;

	if (g_ps.pServiceProvider && g_ps.pRDTEventsListener)
	{
		if (isOpened)
		{
			if (!g_ps.pRDTEventsListener->HasAdvised())
			{
				CComPtr<IVsRunningDocumentTable> pRDT;
				HRESULT hr = g_ps.pServiceProvider->QueryService(SID_SVsRunningDocumentTable, IID_IVsRunningDocumentTable, (void**)&pRDT);
				if (pRDT)
					g_ps.pRDTEventsListener->Advise(pRDT);
			}
		}
		else
			g_ps.pRDTEventsListener->Unadvise();
	}
}


// OnTimer 实现
LRESULT LazyBugPlugInWindowPane::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == IDT_CHATDIALOG_TIMER)
	{
		UpdateEventListener();

		UpdateSolutionDump();

		extern void UpdateFileChangeAttach();
		UpdateFileChangeAttach();

		extern bool UpdateFileLocatorOpenDocument();
		UpdateFileLocatorOpenDocument();

		UpdateChatInputEscape();

		bHandled = TRUE;
		return 0;
	}

	bHandled = FALSE; // 如果不是我们的定时器，则传递给默认处理
	return 0;
}

LRESULT LazyBugPlugInWindowPane::OnForceUpdateFileChangeAttach(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	extern void UpdateFileChangeAttach();
	UpdateFileChangeAttach();

	// 如果消息已处理，设置 bHandled 为 TRUE
	bHandled = TRUE;

	// 返回自定义的结果值
	return 0;
}

