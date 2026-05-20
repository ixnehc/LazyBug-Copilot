// Proj_WorldEditor2.cpp : Defines the class behaviors for the application.
//

#include "stdh.h"

#include "LazyBug.h"

#include "ChildDoc.h"
#include "ChildView.h"
#include "ChildFrm.h"

#include "MainFrm.h"

#include "FileSystem/IFileSystem.h"

#include "engine/Engine.h"

#include "ScintillaWnd.h"


#include "interface/InterfaceInstantiate.h"
#include "Registry/Registry.h"
#include "stringparser/stringparser.h"
#include "Log/LogDump.h"

#include "SolutionDBServer.h"
#include "SolutionDBApi.h"

#include <curl/curl.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef LOCAL_SOLUTIONDB_SERVICE
CSolutionDBService g_solutoinDBService;
#endif

// CLazyBugApp

BEGIN_MESSAGE_MAP(CLazyBugApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
 	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_NEW2, OnFileNew)
END_MESSAGE_MAP()


// CLazyBugApp construction

CLazyBugApp::CLazyBugApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

CCurrentUserRegistry g_reg("IxSoftware","IxEngine");

// const char* GetCurModuleFolderPath()
// {
// 	static std::string path= GetModuleFolderPath(NULL);
// 	return path.c_str();
// }

const char* GetCurModuleFolderPath_utf8()
{
	static std::string pathUtf8;
	if (pathUtf8.empty())
	{
		wchar_t buffer[512];
		GetModuleFileNameW(NULL, buffer, 500);
		std::wstring wstr(buffer);
		// 找到最后一个反斜杠，截断得到目录路径
		size_t lastSlash = wstr.rfind(L'\\');
		if (lastSlash != std::wstring::npos)
		{
			wstr = wstr.substr(0, lastSlash);
		}
		pathUtf8 = widechar_to_utf8(wstr.c_str());
	}
	return pathUtf8.c_str();
}

CLazyBugApp theApp;

// CLazyBugApp initialization

extern "C" AFX_EXT_API void WINAPI InitGuiLib();
BOOL CLazyBugApp::InitInstance()
{
#ifndef _DEBUG
	extern bool InitErrorReport(LPCTSTR StorePath /* = NULL */, bool GenDump2 /* = false */, bool IsSvr /* = false */);
	InitErrorReport(NULL,false,false);
#endif

	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	InitGuiLib();

	AfxOleInit();

	// 初始化CURL库
	curl_global_init(CURL_GLOBAL_ALL);

#ifdef LOCAL_SOLUTIONDB_SERVICE
	g_solutoinDBService.Start();
#endif

	SolutionDB_EnsureConnected();

	if (TRUE)
	{

		EngineParam param;

		if (FALSE==g_Engine.Init(param))
			return FALSE;
	}


	extern GuiLib_Api void SetGuiStrLib(CStrLib *strlib);
	SetGuiStrLib(g_Engine.GetStrLib());

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Passion Entertainment"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	if (TRUE)
	{
		pDocTemplate = new CMultiDocTemplate(
			IDR_PROTOLIBFRAME,
			RUNTIME_CLASS(CChildDoc),
			RUNTIME_CLASS(CChildFrame), // custom MDI child frame
			RUNTIME_CLASS(CChildView));
	}


 	_hDllScintilla= CScintillaWnd::LoadScintillaDll();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
// 	// Dispatch commands specified on the command line.  Will return FALSE if
// 	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
// 	if (!ProcessShellCommand(cmdInfo))
// 		return FALSE;

	if (TRUE)
	{
		m_pMainWnd=new CMainFrame;
		((CMainFrame*)m_pMainWnd)->LoadFrame(IDR_PROTOLIBFRAME,
			WS_POPUP|WS_CAPTION|WS_VISIBLE|WS_SYSMENU|
			WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|FWS_ADDTOTITLE);
		//		_pProtoLibWnd->SetParent(this);

		m_pMainWnd->SetWindowPos(NULL,0,0,700,800,SWP_NOMOVE|SWP_NOZORDER);

		m_pMainWnd->ShowWindow(SW_MAXIMIZE);

		((CMainFrame*)m_pMainWnd)->SetDocTemplate(pDocTemplate);
	}

	m_pMainWnd->UpdateWindow();

//	_Open(g_Engine.GetWS()->GetPath(WSPath_ProtoLib));


	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CLazyBugApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CLazyBugApp message handlers


int CLazyBugApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class


	g_Engine.UnInit();

	SAFE_DELETE(m_pMainWnd);

	if (_hDllScintilla!= NULL)
		AfxFreeLibrary(_hDllScintilla);

	AfxOleTerm();

	curl_global_cleanup();

#ifdef LOCAL_SOLUTIONDB_SERVICE
	g_solutoinDBService.Stop();
#endif

	SolutionDB_Disconnect();


	return CWinApp::ExitInstance();
}

BOOL CLazyBugApp::_Open(const char *path0)
{
	CMainFrame *wnd=(CMainFrame *)AfxGetMainWnd();

	std::string path=path0;

	wnd->OnCloseDocument();

	wnd->OnOpenDocument(path0);

	AddToRecentFileList(path0);

	return TRUE;
}



void CLazyBugApp::OnFileNew()
{
	CMainFrame* wnd = (CMainFrame*)AfxGetMainWnd();

	CXTBrowseDialog dlg;

	dlg.SetOptions(BIF_DONTGOBELOWDOMAIN | BIF_NEWDIALOGSTYLE);
	//	dlg.SetSelPath((char*)pathProtoRoot.c_str());
	if (IDOK != dlg.DoModal())
		return;

	CString selPath = dlg.GetSelPath();

	// 检查目录是否为空目录（没有文件或子目录）
	BOOL isEmpty = TRUE;
	{
		WIN32_FIND_DATA findData;
		HANDLE hFind;
		CString searchPath = selPath;
		searchPath += _T("\\*");
		
		hFind = FindFirstFile(searchPath, &findData);
		
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				// 跳过"."和".."
				if (_tcscmp(findData.cFileName, _T(".")) != 0 && 
					_tcscmp(findData.cFileName, _T("..")) != 0)
				{
					isEmpty = FALSE;
					break;
				}
			} while (FindNextFile(hFind, &findData));
			
			FindClose(hFind);
		}
	}
	
	if (!isEmpty)
	{
		MessageBox(NULL, _T("所选目录为有内容，请选择空目录。"), _T("提示"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	_Open(toMBCS(selPath));
}

void CLazyBugApp::OnFileOpen()
{
	CMainFrame *wnd=(CMainFrame *)AfxGetMainWnd();

	CFileDialog fileBrowse(TRUE, _T("sln"), NULL, 0, _T("visual studio solution file|*.sln|"));

	if (IDOK == fileBrowse.DoModal())
	{
		CString path = fileBrowse.GetPathName();

		_Open(toMBCS(path));
	}

// 	CXTBrowseDialog dlg;
// 
// 	dlg.SetOptions(BIF_DONTGOBELOWDOMAIN | BIF_NEWDIALOGSTYLE);
// //	dlg.SetSelPath((char*)pathProtoRoot.c_str());
// 	if (IDOK!=dlg.DoModal())
// 		return;

}

CDocument* CLazyBugApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	if (_Open(lpszFileName))
		return (CDocument*)1;
	return NULL;
}


BOOL CLazyBugApp::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message==0xc03b)
		return TRUE;
	CWnd *wnd=AfxGetMainWnd();
	BOOL bIsWindow=::IsWindow(wnd->GetSafeHwnd());
	if (!bIsWindow)
		return TRUE;
	return CWinApp::PreTranslateMessage(pMsg);
}

