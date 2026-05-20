#include "stdh.h"
#include "resource.h"
#include "LazyBugPlugInControlsExport.h"

//#include "../Proj_LazyBug/ChatDialog.h"
#include "../Proj_LazyBug/ChatDialogA.h"
// #include "../Proj_LazyBug/ChangelistsDialog.h"
#include "../Proj_LazyBug/Checkpoints.h"
#include "../Proj_LazyBug/BackupDepot.h"
#include "../Proj_LazyBug/Utils.h"
// #include "../Proj_LazyBug/SolutionDB.h"
#include "../Proj_LazyBug/SolutionDBApi.h"

#include "../Proj_LazyBug/CodingHistory.h"
#include "../Proj_LazyBug/SymbolRefsCache.h"

// #include "../Proj_FileSystem/FileSystemExport.h"
// #include "../Interfaces/FileSystem/IFileSystem.h"

#include "stringparser/stringparser.h"

#include "filewatcher/FileWatcher.h"



//CChatDialog* g_chatDlg = NULL;
CChatDialogA* g_chatDlg = NULL;
// CChangelistsDialog* g_changelistsDlg = NULL;

// CChangelists g_changelists;
CCheckpoints g_checkpoints;
CBackupDepot g_backupDepot;

CCodingHistory g_codingHistory;
CSymbolRefsCache g_symbolRefsCache;

CFileWatcher g_fileWatcher;
std::unordered_set<std::string> g_changedFilePathes;
AbsTick g_recentChangedFileTime = 0;


void MfcInit(HINSTANCE hInstance)
{
	// In an MFC Regular DLL (static or dynamic), DllMain automatically initializes MFC.
	// Calling AfxWinInit explicitly here overrides the DLL's resource handle when using Static MFC, 
	// which causes dialog creation to fail because it looks for resources in the caller's hInstance.
	// BOOL bOk=AfxWinInit(hInstance, NULL, NULL, 0);
}


void MfcTerm()
{
	// AfxWinTerm is automatically called by the MFC Regular DLL's DllMain during DLL_PROCESS_DETACH.
	// AfxWinTerm();
}



HWND CreateChatDialog(HWND hParent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

//	g_chatDlg = new CChatDialog;
	g_chatDlg = new CChatDialogA;

	g_chatDlg->Create(CWnd::FromHandle(hParent));
	g_chatDlg->ShowWindow(SW_SHOW);

// 	edit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_BORDER |
// 			ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, CRect(0, 0, 200, 80), CWnd::FromHandle(hParent), 4020);

	return g_chatDlg->GetSafeHwnd();
} 

// 全局函数，供VS扩展调用
void SetFocusToChatInput()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (g_chatDlg->GetParent())
		g_chatDlg->GetParent()->SetFocus();

	if (g_chatDlg)
		g_chatDlg->SetFocusToChatInput();
}



// HWND CreateChangelistsDialog(HWND hParent)
// {
// 	AFX_MANAGE_STATE(AfxGetStaticModuleState());
// 
// 	g_changelistsDlg = new CChangelistsDialog;
// 
// 	g_changelistsDlg->Create(CWnd::FromHandle(hParent));
// 	g_changelistsDlg->ShowWindow(SW_SHOW);
// 
// 	// 	edit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_BORDER |
// 	// 			ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, CRect(0, 0, 200, 80), CWnd::FromHandle(hParent), 4020);
// 
// 	return g_changelistsDlg->GetSafeHwnd();
// }


BOOL PreTranslateMessageToDialog(HWND hDialog, MSG& msg)
{
	if ((g_chatDlg)&&(g_chatDlg->GetSafeHwnd()==hDialog))
		return g_chatDlg->PreTranslateMessage(&msg);
// 	if ((g_changelistsDlg) && (g_changelistsDlg->GetSafeHwnd() == hDialog))
// 		return g_changelistsDlg->PreTranslateMessage(&msg);

	return FALSE;
}

void UpdateUI()
{
// 	if ((g_changelistsDlg)&&(g_changelistsDlg->GetSafeHwnd()))
// 		g_changelistsDlg->UpdateUI();
}


std::string BrowseForFolder(HWND hwndOwner, const char* title)
{
	char path[MAX_PATH] = { 0 };

	// 初始化BROWSEINFO结构体
	BROWSEINFOA bi = { 0 }; // 注意这里使用BROWSEINFOA (ANSI版本)
	bi.hwndOwner = hwndOwner;
	bi.lpszTitle = title;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = NULL;
	bi.lParam = 0;

	// 显示选择文件夹对话框，使用SHBrowseForFolderA (ANSI版本)
	LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);

	if (pidl != nullptr)
	{
		// 获取所选文件夹的路径，使用SHGetPathFromIDListA (ANSI版本)
		if (SHGetPathFromIDListA(pidl, path))
		{
			// 释放PIDL
			CoTaskMemFree(pidl);
			return std::string(path);
		}
		CoTaskMemFree(pidl);
	}

	return std::string(); // 返回空字符串表示取消或出错
}

std::string g_dbFolderPath;//utf8格式字串
std::string g_slnPath;//utf8格式字串

const char* GetOpenedDBFolderPath_utf8()
{
	return g_dbFolderPath.c_str();
}

const char* GetOpenedSlnPath_utf8()
{
	return g_slnPath.c_str();
}

CCheckpoints* GetCheckpoints()
{
	return &g_checkpoints;
}

CBackupDepot* GetBackupDepot()
{
	return &g_backupDepot;
}


void CloseSolution()
{
	g_dbFolderPath = "";
	g_slnPath = "";

	g_checkpoints.Clear();
	g_backupDepot.Clear();
}

void OpenSolution(const char* slnPath)
{
	if (g_slnPath == slnPath)
		return;

	SolutionDB_EnsureConnected();
	

	SolutionDBMsg_Opened opened = SolutionDB_Open(slnPath);
	if (!opened.success)
		return;

	g_dbFolderPath = opened.dbFolderPath;
	g_slnPath = slnPath;

	g_checkpoints.Init(g_dbFolderPath.c_str());
	g_backupDepot.Init(g_dbFolderPath.c_str());
}

const unsigned short* GetFileChangeFullPath(const FileChange* change)
{
	static std::wstring ret;

	if (change)
	{
		ret = utf8_to_widechar(change->lowerCaseFullPath);
		return (const unsigned short*)ret.c_str();
	}

	return (const unsigned short*)L"";
}

const unsigned short* GetFileLocatorFullPath(const FileLocate locator)
{
	static std::wstring ret;

	ret = utf8_to_widechar(locator.filePath);
	return (const unsigned short*)ret.c_str();
}

const unsigned short* FetchFileChangeOpenDocumentRequest()
{
	if (g_chatDlg)
	{
		const FileChange* change = g_chatDlg->GetCheckpointsFileChange().GetFileChange();
		if (change)
		{
			if (g_chatDlg->GetCheckpointsFileChange().FetchNewFileChange() + 2500 > GetAbsTick())//这个时间延迟要大于IsSyncWithDisk()里的延迟
				return GetFileChangeFullPath(change);
		}
	}

	return (const unsigned short*)L"";
}

bool FetchChatInputEscapeRequest()
{
	if (g_chatDlg)
	{
		if (g_chatDlg->FetchEscapeInputRequestTime())
			return true;
	}
	return false;
}


const FileChange* GetSelectedFileChange()
{
	if (g_chatDlg)
		return g_chatDlg->GetCheckpointsFileChange().GetFileChange();

	return nullptr;
}

const unsigned short* FetchFileLocatorOpenDocumentRequest(int* outLine)
{
	if (g_chatDlg)
	{
		FileLocate locate;
		if (g_chatDlg->GetFileLocator().FetchRequest(locate) + 500 > GetAbsTick())
		{
			if (outLine)
			{
				*outLine = locate.fileLoc.lineLoc.line;
			}
			return GetFileLocatorFullPath(locate);
		}
	}

	if (outLine)
	{
		*outLine = 0;
	}
	return (const unsigned short*)L"";
}


void UpdateReload()
{

} 

void AddFileToChat(const unsigned short* fullPath)
{
	if (g_chatDlg)
	{
		g_chatDlg->GetChatInput().WaitTillWebViewReady();
		g_chatDlg->GetChatInput().AddFilePathTag((const wchar_t*)fullPath,true);
	}
}