#include "stdh.h" // Common header, adjust if your project uses a different one

#include "guids.h"
#include "LazyBugChangelistsWindow.h"
#include <vsshell.h> // For IVsShell, IVsUIShell5 etc.

#include "Utils.h"
#include "OpenedDocStates.h"

CFileChangeAttach g_fileChangeAttach;

bool UpdateFileChangeOpenDocument()
{
	const wchar_t* path = FetchFileChangeOpenDocumentRequest();
	if (!path)
		return false;
	if (!path[0])
		return false;
	std::wstring fullPath = path;
	Util_OpenFileInEditor(fullPath.c_str());

	return true;
}

void UpdateFileChangeAttach()
{
	if (!g_fileChangeAttach.IsEmpty())
	{
		FILETIME fileTime;
		fileTime = Util_GetFileTime(g_fileChangeAttach._filePath);
		if (!Util_EqualFileTime(fileTime,g_fileChangeAttach._fileTimeWhenAttach))
			g_fileChangeAttach.Detach();
		else
		{
			if (S_OK != Util_IsFileOpenInEditor(g_fileChangeAttach._filePath.c_str()))
				g_fileChangeAttach.Detach();
		}
	}

	const FileChange* change = GetSelectedFileChange();
	FILETIME fileTimeOfChange;
	if (change)
	{
		std::wstring path = GetFileChangeFullPath(change);
		if (!g_openDocStates.IsSyncWithDisk(path, fileTimeOfChange))
			return;
	}
	if (UpdateFileChangeOpenDocument())
	{
		if (change)
		{
			if (!g_fileChangeAttach.IsEmpty())
			{
				if (change->Equals(g_fileChangeAttach._change))
				{
					Util_NavigateNextDiff(g_fileChangeAttach._filePath, g_fileChangeAttach._comparingContent, true ,true);
					return;
				}
				g_fileChangeAttach.Detach();
			}

			bool enabledWritableTemp = false;
			std::wstring filePath = CFileChangeAttach::GetFullPath(*change);

			if (CFileChangeAttach::CheckReadOnly(filePath))
			{
				CFileChangeAttach::EnableWritable(filePath, true);
				Util_ReloadFile(filePath.c_str());
				enabledWritableTemp = true;
			}

			g_fileChangeAttach.Attach(*change, fileTimeOfChange);

			if (enabledWritableTemp)
				CFileChangeAttach::EnableWritable(filePath, false);
		}
	}

	if ((!change) || g_ps.requestDetachFileChange)
	{
		g_fileChangeAttach.Detach();
		g_ps.requestDetachFileChange = false;
	}
}

bool UpdateFileLocatorOpenDocument()
{
	int line = 0;
	const wchar_t* path = FetchFileLocatorOpenDocumentRequest(&line);
	if (!path)
		return false;
	if (!path[0])
		return false;

	if (line > 0)
		g_fileChangeAttach.Detach();

	std::wstring fullPath = path;
	Util_OpenFileInEditor(fullPath.c_str(), line);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//LazyBugChangelistsWindowPane

STDMETHODIMP LazyBugChangelistsWindowPane::TranslateAccelerator(LPMSG lpMsg)
{
	if (m_hContentDialog&& ::IsWindow(m_hContentDialog))
	{
		if (PreTranslateMessageToDialog(m_hContentDialog, *lpMsg))
		{
			return S_OK;
		}
	}
	return S_FALSE;
}


void LazyBugChangelistsWindowPane::PostSited(IVsPackageEnums::SetSiteResult result)
{
	VsWindowFrameEventSink<LazyBugChangelistsWindowPane>::SetSite(GetVsSiteCache());

	CComPtr<IVsShell> spShell = GetVsSiteCache().GetCachedService<IVsShell, SID_SVsShell>();
	if(spShell)
		spShell->AdviseBroadcastMessages(this, &m_BroadcastCookie);
	InitVSColors();
}

void LazyBugChangelistsWindowPane::PostClosed()
{
	// 停止定时器
	::KillTimer(this->GetHWND(), IDT_CHANGELISTS_TIMER);

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

	// If m_hContentDialog was created and needs cleanup:
	if (::IsWindow(m_hContentDialog))
	{
		::DestroyWindow(m_hContentDialog);
		m_hContentDialog = nullptr;
	}
}

void LazyBugChangelistsWindowPane::OnFrameSize(int x, int y, int w, int h)
{
	// This method is called when the tool window frame is sized.
	// You can use this to resize your m_hContentDialog or other child windows.
	if (m_hContentDialog && ::IsWindow(m_hContentDialog))
	{
		::SetWindowPos(m_hContentDialog, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

LRESULT LazyBugChangelistsWindowPane::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	HWND hParentWnd = this->GetHWND(); // This is the HWND of LazyBugChangelistsWindowPane itself.

// 	m_hContentDialog = CreateChangelistsDialog(hParentWnd);
// 	::SetTimer(this->GetHWND(), IDT_CHANGELISTS_TIMER, 10, nullptr);

	bHandled = TRUE; 
	return FALSE; // Return TRUE if you set the focus, FALSE otherwise.
}

LRESULT LazyBugChangelistsWindowPane::OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (nullptr != m_hBackground)
	{
		bHandled = TRUE;
		return (LRESULT)m_hBackground;
	}
	bHandled = FALSE;
	return 0;
}

LRESULT LazyBugChangelistsWindowPane::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// This handles WM_SIZE for the LazyBugChangelistsWindowPane itself.
	// If you have a child content window (m_hContentDialog), resize it here.
	if (m_hContentDialog && ::IsWindow(m_hContentDialog))
	{
		::SetWindowPos(m_hContentDialog, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOACTIVATE);
	}
	// If LazyBugChangelistsWindowPane's dialog resource itself contains controls that need manual resizing, do it here.

	bHandled = TRUE;
	return 0;
}


STDMETHODIMP LazyBugChangelistsWindowPane::OnBroadcastMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SYSCOLORCHANGE:
	case WM_PALETTECHANGED:
		InitVSColors();
		break;
	}
	return S_OK;
}

void LazyBugChangelistsWindowPane::InitVSColors()
{
}



// OnTimer 实现
LRESULT LazyBugChangelistsWindowPane::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == IDT_CHANGELISTS_TIMER)
	{
// 		UpdateUI();
// 
// 		UpdateReload();

// 		UpdateFileChangeAttach();

		bHandled = TRUE;
		return 0;
	}

	bHandled = FALSE; // 如果不是我们的定时器，则传递给默认处理
	return 0;
} 

