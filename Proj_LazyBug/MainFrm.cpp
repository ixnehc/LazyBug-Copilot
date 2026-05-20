// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdh.h"
#include "commondefines/general_stl.h"


#include "ChildDoc.h"
#include "ChildView.h"
#include "MainFrm.h"

#include "weditor/WEditor.h"


#include "editor/editor.h"
#include "GuiEditor.h"

#include "stringparser/stringparser.h"


#include "WEditor_MainFrame.h"
// #include "WEditor_proto.h"

#include "WMGuiLib.h"

#include "GuiCmd.h"


#include "engine/Engine.h"

#include "Registry/Registry.h"
#include "Log/LogDump.h"
#include "Log/LogFile.h"
#include "datapacket/datapacket.h"

#include "SolutionDBApi.h"
// #include "SolutionDBServer.h"

#include "TreeSitterFiles.h"
#include "SymbolRefsCache.h"
#include "CodingHistory.h"

#include "SolutionDump.h"

#include "LlmSkills.h"
#include "Utils_Skill.h"


extern CCurrentUserRegistry g_reg;

CSmartRepair g_smartRepair;

CTreeSitterFiles g_tsFiles;

CSymbolRefsCache g_symbolRefsCache;

CCodingHistory g_codingHistory;

CMainFrame *g_pMainWnd=NULL;

BOOL CheckForeground(CWnd* wnd)
{
	HWND hWndFore = GetForegroundWindow();
	DWORD idFore, id;
	GetWindowThreadProcessId(hWndFore, &idFore);
	GetWindowThreadProcessId(wnd->GetSafeHwnd(), &id);
	return idFore == id;
}


CWEditor_MainFrame* GetEditorMainFrame()
{
	CMainFrame* pMainFrame = (CMainFrame*)g_pMainWnd;
	if (pMainFrame)
		return pMainFrame->GetEditor();
	return NULL;
}

const char* GetOpenedDBFolderPath_utf8()
{
	CWEditor_MainFrame* editor = GetEditorMainFrame();
	if (!editor)
		return "";
	return editor->GetDBFolderPath();
}

CMainFrame* GetMainFrame()
{
	return (CMainFrame*)g_pMainWnd;
}

// CLspClient* GetLspClient()
// {
// 	CLspClient* lspClient = NULL;
// 	if (GetMainFrame())
// 	{
// 		CWEditor_MainFrame* editor = GetMainFrame()->GetEditor();
// 		if (editor)
// 			lspClient = &editor->GetLspClient();
// 	}
// 	return lspClient;
// }

// CChangelists* GetChangelists()
// {
// 	CChangelists* changelists= NULL;
// 	if (GetMainFrame())
// 	{
// 		CWEditor_MainFrame* editor = GetMainFrame()->GetEditor();
// 		if (editor)
// 			changelists = &editor->GetChangelists();
// 	}
// 	return changelists;
// }

CCheckpoints* GetCheckpoints()
{
	CCheckpoints* checkpoints= NULL;
	if (GetMainFrame())
	{
		CWEditor_MainFrame* editor = GetMainFrame()->GetEditor();
		if (editor)
			checkpoints = &editor->GetCheckpoints();
	}
	return checkpoints;
}

CBackupDepot* GetBackupDepot()
{
	CBackupDepot* backupDepot= NULL;
	if (GetMainFrame())
	{
		CWEditor_MainFrame* editor = GetMainFrame()->GetEditor();
		if (editor)
			backupDepot = &editor->GetBackupDepot();
	}
	return backupDepot;
}

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CXTPMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CXTPMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_COMMAND(XTP_ID_CUSTOMIZE, OnCustomize)
	ON_COMMAND(ID_NEXT_WINDOW,OnNextWindow)
    ON_MESSAGE(XTPWM_DOCKINGPANE_NOTIFY, OnDockingPaneNotify)
	ON_MESSAGE(WM_XTP_PRETRANSLATEMOUSEMSG, OnTabbarMouseMsg)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_COMMAND(ID_WINDOW_CLOSEALL,OnWindowCloseAll)
	ON_COMMAND(ID_SYNC_FILE, OnSyncFile)
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};
#define STATUSID_LUACOST 100

static UINT uHideCmds[] = 
{
	ID_FILE_PRINT,
	ID_FILE_PRINT_PREVIEW,
};

CMainFrame::CMainFrame()
{
	_editor=NULL;
	_viewer=NULL;
	_pDocTemplate=NULL;
	g_pMainWnd =this;
	_bDestroyed=TRUE;

	_bInTimer=FALSE;
	_nTimerCount=0;

	_recentChangedFileInfoTime = 0;
}

CMainFrame::~CMainFrame()
{
	SAFE_DELETE(_pDocTemplate);
}

// void CMainFrame::RegisterLogHandler()
// {
// 	LogHandler handler;
// 	handler.bind(this,&CMainFrame::_HandleLog);
// 
// 	g_Engine.RegisterLogHandler(handler);
// 	::RegisterLogHandler(handler);
// 	GuiLib_Api void RegisterGuiLogHandler(LogHandler handler);
// 	RegisterGuiLogHandler(handler);
// 
// }


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;


	//the timer
	_idTimer=(UINT)SetTimer(0,10,NULL);
	_idTimerLowFreq = (UINT)SetTimer(1, 1000, NULL);

	if (!_wndStatusBar.Create(this) ||
		!_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
    // Initialize the command bars
    if (!InitCommandBars())
        return -1;

    // Get a pointer to the command bars object.
    CXTPCommandBars* pCommandBars = GetCommandBars();
    if(pCommandBars == NULL)
    {
        TRACE0("Failed to create command bars object.\n");
        return -1;      // fail to create
    }

    // Add the menu bar
    CXTPCommandBar* pMenuBar = pCommandBars->SetMenu(
        _T("Menu Bar"), IDR_PROTOLIBFRAME);       
    if(pMenuBar == NULL)
    {
        TRACE0("Failed to create menu bar.\n");
        return -1;      // fail to create
    }

    // Set Office 2003 Theme
    CXTPPaintManager::SetTheme(xtpThemeOffice2003);

	// Hide array of commands
	pCommandBars->HideCommands(uHideCmds, _countof(uHideCmds));

	// Set "Always Show Full Menus" option to the FALSE
	pCommandBars->GetCommandBarsOptions()->bAlwaysShowFullMenus = FALSE;

    // Load the previous state for toolbars and menus.
    LoadCommandBars(_T("CommandBars"));

	CXTPStatusBarPane *pane=_wndStatusBar.AddIndicator(STATUSID_LUACOST,1);
	pane->SetWidth(200);

	// Initialize the docking pane manager and set the
	// initial them for the docking panes.  Do this only after all
	// control bars objects have been created and docked.
	_paneManager.InstallDockingPanes(this);
    // Set Office 2003 Theme
	_paneManager.SetTheme(xtpPaneThemeOffice2003);

	VERIFY(m_MTIClientWnd.Attach(this, TRUE));
	m_MTIClientWnd.EnableToolTips();

// 	if (TRUE)
// 	{
// 		CXTPDockingPane* panel = _paneManager.CreatePane(IDR_CHANGELISTS_PANEL, CRect(0, 0, 200, 120), xtpPaneDockBottom);
// 		panel->SetOptions(xtpPaneNoCloseable| xtpPaneNoFloatable);
// //		panel->SetOptions(xtpPaneNoFloatable );
// 		_changelistsDlg.Create(this);
// 		_changelistsDlg.ShowWindow(SW_HIDE);
// // 		panel->Attach(&_changelistsDlg);
// 	}

	if (TRUE)
	{
		CXTPDockingPane* panel = _paneManager.CreatePane(IDR_CHAT_PANEL, CRect(0, 0, 200, 120), xtpPaneDockBottom);
		panel->SetOptions(xtpPaneNoCloseable | xtpPaneNoFloatable);
//		panel->SetOptions(xtpPaneNoFloatable);
		_chatDlg.Create(this);
		_chatDlg.ShowWindow(SW_HIDE);
//		panel->Attach(&_chatDlg);
	}

	g_tsFiles.Init();
	g_smartRepair.Init(this);

	_editor=new CWEditor_MainFrame;
	_editor->Create(&_paneManager);
// 	GStubConnect(&_editor->_panelPrl,DblClickProto,this,OpenProto);
// 	if (!g_bWE)
// 		GStubConnect(&_editor->_panelPrl,LibModified,this,RefreshAllProto);

	//the editor environment
	CWorldEditor::SetEngineSS();


	//Load the previous state for docking panes.
	CXTPDockingPaneLayout layoutNormal(&_paneManager);
	if (layoutNormal.Load(_T("LazyBug003")))
	{
		_paneManager.SetLayout(&layoutNormal);
	}

	//注册Panel viewer
	_viewer=new CPanelViewer;
	_viewer->SetMgr(&_paneManager);
	_viewer->Register(IDR_EDITORPANEL_DB,IDR_EDITORPANEL_DB);
	_viewer->Register(IDR_CHANGELISTS_PANEL, IDR_CHANGELISTS_PANEL);
	_viewer->Register(IDR_CHAT_PANEL, IDR_CHAT_PANEL);

	// 加载图标
#ifdef LAZYBUG_RETAIL
	HICON hIcon = AfxGetApp()->LoadIcon(IDI_LAZYBUG2);
	if (hIcon)
	{
		SetIcon(hIcon, TRUE);   // 设置大图标
		SetIcon(hIcon, FALSE);  // 设置小图标
	}
#endif

	_bDestroyed=FALSE;

// 	if (!g_bWE)
// 		_watcher.Init(g_Engine.GetFS());


	return 0;
}

void CMainFrame::OnNextWindow()
{
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.hwndParent=AfxGetMainWnd()->m_hWnd;

	cs.style |= WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
	cs.dwExStyle&=~WS_EX_OVERLAPPEDWINDOW;
	return TRUE;
}

// CMainFrame message handlers

void CMainFrame::OnDestroy()
{
	_bDestroyed=TRUE;

	m_MTIClientWnd.Detach();

	OnCloseDocument();

	// Save the current state for toolbars and menus.
	SaveCommandBars(_T("CommandBars"));

	// Save the current state for docking panes.
	CXTPDockingPaneLayout layoutNormal(&_paneManager);
	_paneManager.GetLayout(&layoutNormal); 
	layoutNormal.Save(_T("LazyBug003"));


	CXTPMDIFrameWnd::OnDestroy();

	_editor->Destroy();
	SAFE_DELETE(_editor);

	SAFE_DELETE(_viewer);

	g_tsFiles.Clear();

}


void CMainFrame::OnClose() 
{
	CFrameWnd::OnClose();
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
// 	if (!IsDebuggerRunning())
// 		AfxSetWindowText(m_hWnd,"ProtoEditor");
// 	else
// 		AfxSetWindowText(m_hWnd,"ProtoEditor  - [运行模式 (按F6退出) ]  ");

}



void CMainFrame::OnCustomize()
{
    // Get a pointer to the command bars object.
    CXTPCommandBars* pCommandBars = GetCommandBars();
    if(pCommandBars != NULL)
    {
        // Instanciate the customize dialog object.
        CXTPCustomizeSheet dlg(pCommandBars);

        // Add the options page to the customize dialog.
        CXTPCustomizeOptionsPage pageOptions(&dlg);
        dlg.AddPage(&pageOptions);

        // Add the commands page to the customize dialog.
        CXTPCustomizeCommandsPage* pCommands = dlg.GetCommandsPage();
        pCommands->AddCategories(IDR_MAINFRAME);

        // Use the command bar manager to initialize the 
        // customize dialog.
        pCommands->InsertAllCommandsCategory();
        pCommands->InsertBuiltInMenus(IDR_MAINFRAME);
        pCommands->InsertNewMenuCategory();

        // Dispaly the dialog.
        dlg.DoModal();
    }
}

LRESULT CMainFrame::OnDockingPaneNotify(WPARAM wParam, LPARAM lParam)
{
    if (wParam == XTP_DPN_SHOWWINDOW)
    {
        CXTPDockingPane* pPane = (CXTPDockingPane*)lParam;
        
		if (!pPane->IsValid())
		{
			_editor->OnDockPanel(pPane);
			// 			for (int i=0;i<_views.size();i++)
			// 				_views[i].editor->OnDockPanel(pPane);
// 			if (pPane->GetID() == IDR_CHANGELISTS_PANEL)
// 			{
// 				if (!_changelistsDlg.GetSafeHwnd())
// 				{
// 					_changelistsDlg.Create(this);
// 				}
// 				pPane->Attach(&_changelistsDlg);
// 				_changelistsDlg.ShowWindow(SW_SHOW);
// 			}
			if (pPane->GetID() == IDR_CHAT_PANEL)
			{
				if (!_chatDlg.GetSafeHwnd())
				{
					_chatDlg.Create(this);
				}
				pPane->Attach(&_chatDlg);
				_chatDlg.ShowWindow(SW_SHOW);
			}
		}

        return TRUE;
    }
    return FALSE;
}

void CMainFrame::OnOpenDocument(const char* pathSln)
{
	CWorldEditor::SetWorldSS();

	SolutionDB_EnsureConnected();

	SolutionDBMsg_Opened opened = SolutionDB_Open(pathSln);
	if (!opened.success)
		return;

	SolutionDump slnDmp;
	if (Utils::GenerateSolutionDump(opened.dbFolderPath.c_str(),pathSln, slnDmp))
	{
		std::string path;
		path = opened.dbFolderPath + "\\.slndmp";
		extern void SaveSolutionDump(const char* fullPath, SolutionDump & slnDump);
		SaveSolutionDump(path.c_str(), slnDmp);
	}

	_editor->LoadContent(opened.dbFolderPath.c_str());

	Utils::LoadLlmSkills(g_llmSkills, opened.dbFolderPath.c_str());

	g_symbolRefsCache.Init(opened.dbFolderPath.c_str());

	// 初始化LSP客户端
// 	_editor->InitLspClient();
	
	if (_fileWatcher.IsStarted())
		_fileWatcher.Stop();
//	_fileWatcher.Start(_editor->GetWorkspacePath());

// 	_changelistsDlg.SetChangelists(&_editor->GetChangelists());
}

void CMainFrame::OnCloseDocument()
{
	//先关闭所有的子窗口
	OnWindowCloseAll();

	if (_editor)
		_editor->ResetContent();

 	_fileWatcher.Stop();

	g_symbolRefsCache.Clear();

// 	for (int i=0;i<_views.size();i++)
// 	{
// 		_views[i].editor->Destroy();
// 		SAFE_DELETE(_views[i].editor);
// 	}
	_views.clear();

}


BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	return CXTPMDIFrameWnd::OnCreateClient(lpcs,pContext);
	return TRUE;
}


void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CXTPMDIFrameWnd::OnSize(nType,cx,cy);
}

void CMainFrame::_DoTimer(CWorldEditor *editor)
{
	if (!editor)
		return;
	CGuiMgr *mgr=editor->GetMgr();
	if (!mgr)
		return;
	mgr->Update();
	mgr->RedrawView();

//	_changelistsDlg.UpdateUI();
//	_chatDlg.UpdateUI();

// 	_UpdateRegistryEvents();
}

BOOL CMainFrame::_DoCmdMsg(CWorldEditor *editor,UINT nID,int nCode,void *extra)
{
	//Let the viewer try
	if (_viewer)
	{
		if (nCode==CN_UPDATE_COMMAND_UI)
		{
			if(_viewer->Handle(nID,TRUE))
			{
				((CCmdUI*)extra)->Enable(TRUE);
				return TRUE;
			}
		}
		else
		{
			if (_viewer->Handle(nID,FALSE))
				return TRUE;
		}
	}

	if (!editor)
		return FALSE;
	CGuiMgr *mgr=editor->GetMgr();
	if (!mgr)
		return FALSE;

	GuiCmd cmd=GuiCmd_Max;

	//map menu command id to GuiCmd
	switch(nID)
	{
	case ID_EDIT_UNDO:
		cmd=GuiCmd_Undo;
		break;
	case ID_EDIT_REDO:
		cmd=GuiCmd_Redo;
		break;
	}

	if (cmd<GuiCmd_Max)
	{
		if (nCode==CN_UPDATE_COMMAND_UI)
			mgr->UpdateCommandUI(cmd,extra);
		if (nCode==CN_COMMAND)
			mgr->DoCommand(cmd);
		return TRUE;
	}

	return FALSE;
}

CChildView *CMainFrame::_GetActiveView()
{
	CView *view=NULL;

	CFrameWnd *frame=GetActiveFrame();
	if (frame)
		view=frame->GetActiveView();
	while(view)
	{
		int idx;
		VEC_FIND_BY_ELEMENT(_views,view,view,idx);
		if (idx!=-1)
			return (CChildView*)view;
		view=(CView *)(((CWnd*)view)->GetParent());
	}

	return NULL;

}


void CMainFrame::_UpdateActiveActor()
{
}



void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
 	if (_bDestroyed)
 		return;
	if (_bInTimer)
		return;//to avoid recursive calling

	if (nIDEvent == _idTimerLowFreq)
	{
		_bInTimer = TRUE;

// 		_editor->GetLspClient().RequestWorkspaceSymbols(std::string("a"));

		_bInTimer = FALSE;
		return;
	}

#ifdef LOCAL_SOLUTIONDB_SERVICE
	extern CSolutionDBService g_solutoinDBService;
	g_solutoinDBService.Update();
#endif

	SolutionDB_EnsureConnected();

// 	if (!CheckForeground(this))
// 		return;


	_bInTimer=TRUE;

// 	if (!g_bWE)
// 		_watcher.Update();

	UpdateOpenSelectFile_DBPanel();
	UpdateOpenSelectFile_Changelists();
	UpdateFileLocate();

	_UpdateGoToDefination();

	if (_editor)
		_editor->Update();

//	_editor->_panelDB.UpdateUI();

	_DoTimer(_editor);
// 	for (int i=0;i<_views.size();i++)
// 	{
// 		if (_views[i].editor->IsEnable())
// 		{
// 			_DoTimer(_views[i].editor);
// 			
// 		}
// 	}

	_UpdateReload();

	_bInTimer=FALSE;

	_UpdateActiveActor();

}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (_DoCmdMsg(_editor,nID,nCode,pExtra))
		return TRUE;

	CChildView *view=_GetActiveView();

	if (view)
	{
		if (view->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
			return TRUE;
	}

	return CXTPMDIFrameWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}


void CMainFrame::OnWindowCloseAll()
{
	if (TRUE)
	{
		CMDIChildWnd *child;
		while(child=MDIGetActive())
			child->SendMessage(WM_CLOSE,0,0);
	}
}


void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CXTPMDIFrameWnd::OnActivate(nState, pWndOther, bMinimized);
}

void CMainFrame::UpdateOpenSelectFile_DBPanel()
{
//     if (!_editor || !_editor->_panelDB.GetSafeHwnd())
//     {
//         return;
//     }
// 
// 	if (_editor->_panelDB.GetTree().FetchOpenSelRequest() + 500 <= GetAbsTick())
// 		return;
// 
//     // 获取当前选中的虚拟路径
//     std::string virtualPath = _editor->_panelDB.GetCurSelPath();
//     if (virtualPath.empty())
//     {
//         return;
//     }
// 
//     // 在数据库中查找对应的实际路径
//     const std::unordered_map<std::string, CVcxprojEntries::Entry>& entries = _editor->_db._entries._entries;
//     auto it = entries.find(virtualPath);
//     if (it == entries.end())
//     {
//         return;
//     }
// 
//     // 获取实际文件路径(相对路径)
//     std::string realPath = it->second.pathReal;
// 
//     // 获取数据库的workspace路径
//     std::string workspacePath = _editor->_db._setting.pathWorkspace;
//     if (workspacePath.empty())
//     {
//         return;
//     }
// 
//     // 构建完整的绝对路径
//     std::string fullPath = workspacePath + "\\" + realPath;
// 	fullPath = ResolveRelativePathWithDots(fullPath);
// 
// 	_OpenView(fullPath.c_str(),realPath.c_str());
}

void CMainFrame::UpdateOpenSelectFile_Changelists()
{
// 	if (!_editor || !_editor->_panelDB.GetSafeHwnd())
// 		return;
// 
// 	if (!_changelistsDlg.GetSafeHwnd())
// 		return;
// 
// 	// 获取数据库的workspace路径

	bool needDetach = true;
	const FileChange* changeToOpen = nullptr;
// 
// 	if (!changeToOpen)
// 	{
// 		const FileChange* change = _changelistsDlg.GetSelectedFileChange();
// 		if (change)
// 		{
// 			if (_changelistsDlg.FetchOpenSelRequest() + 500 > GetAbsTick())
// 				changeToOpen = change;
// 			needDetach = false;
// 		}
// 	}
// 
	if (!changeToOpen)
	{
		const FileChange* change = _chatDlg.GetCheckpointsFileChange().GetFileChange();
		if (change)
		{
			if (_chatDlg.GetCheckpointsFileChange().FetchNewFileChange() + 500 > GetAbsTick())
				changeToOpen = change;
			needDetach = false;
		}
	}

	if (changeToOpen)
	{
		// 构建完整的绝对路径
		std::string fullPath = changeToOpen->lowerCaseFullPath;

		_OpenView(fullPath.c_str(), changeToOpen);
		return;
	}

	if (needDetach)
		_DetachChangeOfAllViews();
}


void CMainFrame::UpdateFileLocate()
{
	FileLocate locate;
	if (_chatDlg.GetFileLocator().FetchRequest(locate) + 500 > GetAbsTick())
	{
		CChildView* view = _OpenView(locate.filePath.c_str(), NULL);
		// 如果有行号信息，跳转到指定行
		if (view && locate.fileLoc.lineLoc.line != 0xffff)
		{
			LspRange range;
			range.start.line = locate.fileLoc.lineLoc.line;
			range.start.character = locate.fileLoc.lineLoc.startColumn;
			range.end.line = locate.fileLoc.lineLoc.line;
			range.end.character = locate.fileLoc.lineLoc.endColumn;
			view->RequestSelect(range);
		}
	}
}

void CMainFrame::_UpdateGoToDefination()
{
	if (!_definitionToGo.IsEmpty())
	{
		LspLocation loc = _definitionToGo.locations[0];
		_definitionToGo.Reset();
		std::string path = UriToFilePath(loc.uri);
		CChildView *view=_OpenView(path.c_str(), NULL);
		if (view)
			view->RequestSelect(loc.range);
	}
}


void CMainFrame::_DetachChangeOfAllViews()
{
	for (const auto& viewInfo : _views)
	{
		if (viewInfo.view)
			viewInfo.view->DetachChange();
	}

}


CChildView* CMainFrame::_OpenView(const char* fullPath0, const FileChange* fileChange)
{
	extern std::string GetFinalFilePath(const char* inputPath);
	std::string fullPath = GetFinalFilePath(fullPath0);
    // 检查文件是否已经被打开
    for (const auto& viewInfo : _views)
    {
        if (viewInfo.view && viewInfo.view->GetDocument())
        {
            CChildDoc* pDoc = (CChildDoc*)viewInfo.view->GetDocument();
            if (pDoc->GetPathName() == fullPath.c_str())
            {
				if (fileChange)
					viewInfo.view->AttachChange(*fileChange);

                // 如果文件已经打开,则激活对应的窗口
                MDIActivate(((CWnd*)viewInfo.view)->GetParent());
                return viewInfo.view;
            }
        }
    }

    // 如果文件未打开,则打开它
    CDocument* pDoc = _pDocTemplate->OpenDocumentFile(CString(fullPath.c_str()));

    if (pDoc)
    {
        // 获取新创建的视图
        CView* pView = NULL;
		POSITION pos = pDoc->GetFirstViewPosition();
		if (pos)
        {
            pView = pDoc->GetNextView(pos);
        }

        if (pView)
        {
			if (fileChange)
				((CChildView*)pView)->AttachChange(*fileChange);

            // 创建视图信息
            _ViewInfo viewInfo;
            viewInfo.view = (CChildView*)pView;
            viewInfo.editor = NULL;  // 暂时不需要editor

            // 添加到视图列表中
            _views.push_back(viewInfo);

            // 激活新窗口
            MDIActivate(((CWnd*)pView)->GetParent());
			return ((CChildView*)pView);
		}
    }

    return NULL;
}


LRESULT CMainFrame::OnTabbarMouseMsg(WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

void CMainFrame::_UpdateReload()
{
	const ChangedFileInformation* infos;
	int nInfo = _fileWatcher.FetchChangedFiles(infos);

	for (int i = 0;i < nInfo;i++)
	{
		const ChangedFileInformation& info = infos[i];
		ChangedFileInformation2 info2;
		info2.action = info.action;
		info2.name = std::string(_fileWatcher.GetWatchDirectory()) + "\\" + info.name;
		StringLower(info2.name);
		_changedFileInfos.push_back(info2);
	}
	if (nInfo > 0)
		_recentChangedFileInfoTime = GetAbsTick();

	AbsTick curTime = GetAbsTick();
	if ((curTime>_recentChangedFileInfoTime+500)||//500ms内没有任何修改信息
		(_changedFileInfos.size()>2048))//累积太多了
	{
		std::unordered_set<std::string> reloads;
		std::unordered_set<std::string> closes;
		std::unordered_set<std::string> vcxprojs;
		while (_changedFileInfos.size() > 0)
		{
			ChangedFileInformation2 info = _changedFileInfos[0];
			_changedFileInfos.pop_front();

			if (CheckFileSuffix(info.name.c_str(), "VCXPROJ"))
				vcxprojs.insert(info.name);
			else
			{
				bool needReload = false;
				bool needClose = false;
				switch (info.action)
				{
					case FA_MODIFIED:
					case FA_RENAMED_NEW_NAME:
					{
						needReload = true;
						break;
					}
					case FA_REMOVED:
					case FA_RENAMED_OLD_NAME:
					{
						needClose = true;
						break;
					}
				}
				if (needClose)
				{
					closes.insert(info.name);
					auto it = reloads.find(info.name);
					if (it != reloads.end())
						reloads.erase(it);
				}
				if (needReload)
				{
					reloads.insert(info.name);
					auto it = closes.find(info.name);
					if (it != closes.end())
						closes.erase(it);
				}
			}
 		}

		for (auto it = _views.begin(); it != _views.end();)
		{
			CChildView* view = it->view;
			if (view && view->GetDocument())
			{
				CChildDoc* pDoc = (CChildDoc*)view->GetDocument();
				std::string path = pDoc->GetPathName();
				StringLower(path);
				if (reloads.find(path)!=reloads.end())
					view->ReloadContent();
				else
				{
					if (closes.find(path) != closes.end())
					{
						view->GetParent()->SendMessage(WM_CLOSE, 0, 0);
						it = _views.erase(it);
						continue;
					}
				}
			}
			++it;
		}

		// 如果是.vcxproj文件发生变化,需要重新加载数据库
// 		if (isVcxprojFile && (needReload || needClose))
// 		{
// 			std::string pathDB = _editor->_db._setting.pathWorkspace + "\\.db";
// 			_editor->LoadContent(pathDB.c_str());
// 		}
	}

	if (nInfo > 0)
	{
// 		_editor->GetChangelists().RefreshCur();
// 		_changelistsDlg.Redraw();
	}
 }

void CMainFrame::OnSyncFile()
{
	CString path;
	CChildView* view = _GetActiveView();
	if (view)
	{
		if (view->GetDocument())
			path = view->GetDocument()->GetPathName();
	}

	if (path.IsEmpty())
		return;

// 	if (_editor->GetPanelDB().GetTree().EnsureVisible((LPCTSTR)path))
// 	{
// 		_paneManager.ShowPane(IDR_EDITORPANEL_DB);
// 	}

}
