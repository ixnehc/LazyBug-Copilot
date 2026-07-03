#include "stdh.h"
#include "ChangelistsDialog.h"
// #include "WndBase.h"

// #include "TreeCtrlBase.h"

#include "stringparser/stringparser.h"

#include "Registry/Registry.h"

//////////////////////////////////////////////////////////////////////////
//CVSColorPage

extern CCurrentUserRegistry g_reg;


//////////////////////////////////////////////////////////////////////////
//CChangelistsDialog
CChangelistsDialog::CChangelistsDialog( CWnd* pParent /* = NULL  */ )
	:CDialog( CChangelistsDialog::IDD, pParent )
{
}



void CChangelistsDialog::Create(CWnd *parent)
{
	CDialog::Create(CChangelistsDialog::IDD,parent);
}


BOOL CChangelistsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
// 	HIDE_CONTROL( this, 1000);
// 	HIDE_CONTROL( this, IDC_TREE );
	//创建窗口
	CRect rc(0, 0, 1, 1);
	_editor.Create(rc, this, 1);
	_editor.ShowWindow(SW_SHOW);
	_editor.EnableWindow(TRUE);

	//GuiEditor
	_mgr.AddModMgr( "Changelists" );
	_mgr.RegisterData( &_data);
	_mgr.RegisterView( &_view);
	_mgr.RegisterActor( &_actor);
	_actor.Reset();

	_editor.SetView(&_view);
	_editor.EnableWindow();

	return TRUE;
}


void CChangelistsDialog::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP(CChangelistsDialog, CDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


void CChangelistsDialog::SetChangelists(CChangelists* changelists)
{
	_data.Set(changelists);
	_view.Invalidate();
	_view.SetNeedReCenterCur();
}


void CChangelistsDialog::_RecalcLayout()
{
	CRect rc;
	GetClientRect((CRect*)&rc);

// 	rc.inflate(-2, -2, -2, -2);

	_editor.MoveWindow(rc);
}


void CChangelistsDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType,cx,cy);

	_RecalcLayout();
}


void CChangelistsDialog::OnDestroy()
{
	_data.Clear();
	_mgr.Reset();
}


void CChangelistsDialog::SetOwnerName(const char *name)
{
}


void CChangelistsDialog::UpdateUI()
{
	if (!m_hWnd)
		return;

	_mgr.RedrawView();
}

BOOL CChangelistsDialog::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message==WM_KEYDOWN)
		return FALSE;
	return CDialog::PreTranslateMessage(pMsg);
}

const FileChange* CChangelistsDialog::GetSelectedFileChange()
{
    // 获取选中的节点UID和文件索引
    FileChangeListUID selectedNodeUID = _data._snapshot._selectedNodeUID;
    int selectedFileIndex = _data._snapshot._selectedFileIndex;
    
    // 如果没有选中任何文件，返回nullptr
    if (selectedNodeUID == FileChangeListUID_Invalid || selectedFileIndex < 0)
        return nullptr;
        
    // 查找对应的changelist
    auto it = _data._changelists->_briefs.find(selectedNodeUID);
    if (it == _data._changelists->_briefs.end())
        return nullptr;
        
    const FileChangelist& cl = it->second;
    
    // 检查文件索引是否有效
    if (selectedFileIndex >= cl.changes.size())
        return nullptr;

	_data._changelists->EnsureFullLoaded(selectedNodeUID);
        
    // 返回选中的文件变更信息
    return &cl.changes[selectedFileIndex];
}

