// MainFrm.h : interface of the CMainFrame class
//


#pragma once
#include "WEditor/DockPaneWnd.h"

#include "ChildDoc.h"
#include "ChildView.h"
#include "ChildFrm.h"

#include "ScintillaFind.h"

#include "MDICycler.h"

#include "StrLibWatcher.h"

// #include "ChangelistsDialog.h"
//#include "ChatDialog.h"
#include "ChatDialogA.h"

#include "SmartRepair.h"

#include "LspClient.h"

struct Log;


class CWorldEditor;
class CWEditor_MainFrame;
class CWEditor_ChildFrame;
class CPanelViewer; 
class CChildView;
class CScintillaFind;
struct FindCmd;
class CMainFrame : public CXTPMDIFrameWnd
{
	
public: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:
	void UpdateOpenSelectFile_DBPanel();
	void UpdateOpenSelectFile_Changelists();
	void UpdateFileLocate();

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	void OnOpenDocument(const char *path);
	void OnCloseDocument();

	CChildView* GetActiveView()
	{
		return _GetActiveView();
	}


	void SetDocTemplate(CMultiDocTemplate* t)	{		_pDocTemplate=t;	}
	CMultiDocTemplate*GetDocTemplate()	{		return _pDocTemplate;	}

	CWEditor_MainFrame* GetEditor()	{		return _editor;	}

	void SetDefiniationToGo(const LspGotoDefinationResult& definition)	{		_definitionToGo = definition;	}

// 	CWEditor_ChildFrame *CreateProtoEditor(ProtoID protoid,CChildView *view);//proto在lib里面的路径(不是文件路径)
// 	void DestroyProtoEditor(CWEditor_ChildFrame *editor,CChildView *view);


// 	//CPrlFrameProxy implement
// 	virtual void EnableAllLuaSrc(BOOL bEnable);
// 	virtual void UpdateLuaSrcToProto(IProto *proto);
// 	virtual void GotoLuaSrc(ProtoID protoid,ProtoNodeID nodeid,int iLine=0);
// 	virtual int FindLuaSrcFunc(ProtoID protoid,ProtoNodeID nodeid,const char *nameFunc);
// 	virtual int FindLuaSrcVar(ProtoID protoid,ProtoNodeID nodeid,const char *nameVar);
// 	virtual void AddLuaSrcFunc(ProtoID protoid,ProtoNodeID nodeid,const char *nameFunc);
// 	virtual BOOL GotoAppearance(ProtoID protoid);
// 	virtual BOOL GotoLogic(ProtoID protoid);
// 	virtual void ClearDebugOutput();
// 	virtual void ShowLuaHelp(BOOL bShow);
// 	virtual void SetHelpKey(const char *func);
// 	virtual const char *GetStartMain();
// 	virtual ProtoID GetActiveProto();
// 	virtual BOOL IsProtoOpened(ProtoID protoid);


// Implementation
public:
	virtual ~CMainFrame();

protected:  // control bar embedded members
	CChildView* _OpenView(const char *fullPath,const FileChange *fileChange=nullptr);
	void _DetachChangeOfAllViews();

	void _UpdateReload();
	void _UpdateGoToDefination();

	CChildView *_GetActiveView();

// 	CChildView *_FindView(ChangelistID idChangelist);

	void _DoTimer(CWorldEditor *editor);
	BOOL _DoCmdMsg(CWorldEditor *editor,UINT nID,int nCode,void *extra);

	void _UpdateActiveActor();


	struct _ViewInfo
	{
// 		ChangelistID idChangeList;
		CChildView* view;
		CWEditor_ChildFrame * editor;
		_ViewInfo &operator=(const _ViewInfo &src)
		{
//			idChangeList = src.idChangeList;
			view = src.view;
			editor = src.editor;
			return *this;
		}
	};

	void _OutputFind(FindCmd &cmd,ProtoID protoid);

	CXTPTabClientWnd m_MTIClientWnd;


	CXTPStatusBar  _wndStatusBar;
	CXTPDockingPaneManager _paneManager;

	BOOL _bInTimer;

	UINT _idTimer;
	UINT _idTimerLowFreq;

	int _nTimerCount;

// 	CDockPaneWnd < CChangelistsDialog> _changelistsDlg;
	CDockPaneWnd < CChatDialogA> _chatDlg;
//	CDockPaneWnd < CChatDialog> _chatDlg;

	CWEditor_MainFrame *_editor;

 	CFileWatcher _fileWatcher;
	std::deque<ChangedFileInformation2> _changedFileInfos;
	AbsTick _recentChangedFileInfoTime;

	LspGotoDefinationResult _definitionToGo;

	CPanelViewer *_viewer;
	std::vector<_ViewInfo> _views;

	CMultiDocTemplate* _pDocTemplate;

	CMDICycler _cycler;

	BOOL _bDestroyed;


	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,AFX_CMDHANDLERINFO* pHandlerInfo);


// 	GStubBegin(CMainFrame);
		
// 		GStubString(OpenProto,"",GSem_Unknown,"");
// 			GStubSetType(GStub_Slot);
// 		GStubVoid(RefreshAllProto,"");
// 			GStubSetType(GStub_Slot);

// 	GStubEnd();


// 	BOOL prop_OpenProto(BOOL bSet,const char *&path);
// 	BOOL prop_RefreshAllProto(BOOL bSet);

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnCustomize();
	afx_msg LRESULT OnDockingPaneNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnWindowCloseAll();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg LRESULT OnTabbarMouseMsg(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNextWindow();
	afx_msg void OnSyncFile();
};


