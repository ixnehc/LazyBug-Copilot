/********************************************************************
	created:	2006/9/1   14:37
	filename: 	e:\IxEngine\Proj_GuiLib\ResEditPanel.cpp
	author:		ixnehc
	
	purpose:	resource edit panel base class
*********************************************************************/
#include "stdh.h"
#include ".\ResEditPanel.h"

#include "commondefines/general_stl.h"

#include "RenderSystem/IUtilRS.h"
#include "RenderSystem/IRenderSystem.h"
#include "FileSystem/IFileSystem.h"

#include "resdata/ResData.h"

#include "stringparser/stringparser.h"

#include "SlideTab.h"

#include "ResTree.h"
#include "ResEditCtrl.h"

#include "WMGuiLib.h"

#include <assert.h>

#include "stringparser/stringparser.h"
#include "resdata/ResDataDefines.h"

#include "GuiAgent_general.h"
#include "GuiEditor_res.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
//CModRes

CModRes::~CModRes()
{
	if (_state)
		_state->CleanAndDelete();
	_state=NULL;
}


BOOL CModRes::TestUndo()
{
	if (!_panel)
		return FALSE;
	if (!_state)
		return FALSE;
	FileAttr attr=g_ssGuiLib.pFS->GetFileAttrAbs(_panel->GetAnchor()->GetPath());
	if (attr==File_ReadOnly)
		return FALSE;//文件不可写

	return TRUE;
}

BOOL CModRes::TestRedo()
{
	return TestUndo();
}


BOOL CModRes::Undo()
{
	if (_state&&_panel)
	{
		if (TRUE)//交换_panel->_stateToMod和_state的内容
		{
			ResEditPanelState*t=_panel->NewState();
			t->Copy(*_panel->_stateToMod);
			_panel->_stateToMod->Copy(*_state);
			_state->Copy(*t);
			t->CleanAndDelete();
		}

		_panel->_stateBackup->Copy(*_panel->_stateToMod);
		_panel->StateToControl(_panel->_stateToMod);
		ResEditPanelState *stateToSave=_panel->_GetStateToSave();
		_panel->StateToFile(stateToSave);

		if (stateToSave!=_panel->_stateToMod)
		{
			if (stateToSave)
				stateToSave->CleanAndDelete();
		}


		return TRUE;
	}
	return FALSE;
}

BOOL CModRes::Redo()
{
	return Undo();
}



//////////////////////////////////////////////////////////////////////////
//ResEditPanelState

void ResEditPanelState::CleanAndDelete()	
{	
	ResData_Delete(resdata);
	Zero();
	delete this;	
}
void ResEditPanelState::Copy(ResEditPanelState &src)
{
	ResData_Delete(resdata);
	resdata=ResData_Clone(src.resdata);
	panel=src.panel;
}
void ResEditPanelState::SetData(ResData *data)
{
	ResData_Delete(resdata);
	resdata=ResData_Clone(data);
}


//////////////////////////////////////////////////////////////////////////
//CResEditPanel

CResEditPanel::CResEditPanel():
	_anchor(Res_None,"None")
{
	m_hIcon = NULL;

	_view=NULL;
	_view2=NULL;

	_tabctrl=NULL;
	_pThisItem=NULL;

	_stateBackup=NULL;
	_stateToMod=NULL;

	_bEnable=TRUE;
}


BEGIN_MESSAGE_MAP(CResEditPanel, CSlidePanel)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL CResEditPanel::Create(CSlideTab *tabctrl,const char *name,int idxIcon)
{
	if (FALSE==CSlidePanel::Create(GetIDD(),tabctrl))
		return FALSE;
	_tabctrl=tabctrl;
	_pThisItem = _tabctrl->InsertItem(0, fromMBCS(name), this, idxIcon);

	return TRUE;
}


// CResEditPanel 消息处理程序

BOOL CResEditPanel::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	_stateToMod=NewState();

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CResEditPanel::BeginBackup()
{
	if (_stateBackup)
		_stateBackup->CleanAndDelete();
	_stateBackup=NewState();
	_stateBackup->Copy(*_stateToMod);
}


// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CResEditPanel::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CResEditPanel::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
 

void CResEditPanel::OnDestroy()
{
	if (_stateToMod)
		_stateToMod->CleanAndDelete();
	_stateToMod=NULL;

	if (_stateBackup)
		_stateBackup->CleanAndDelete();
	_stateBackup=NULL;

	_ctrls.clear();
	_lockedctrl.clear();

	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}

void CResEditPanel::OnClose()
{

	CDialog::OnClose();
}


//if anchor's path is "",will do nothing and return TRUE
BOOL CResEditPanel::_SaveAnchorData(CResAnchor &anchor,ResData *data)
{
	if (!g_ssGuiLib.pUtilRS)
		return FALSE;

	std::string path;
	path=anchor.GetPath();
	if (path=="")
		return TRUE;

	return g_ssGuiLib.pUtilRS->SaveRes(path.c_str(),data);
}

ResEditPanelState *CResEditPanel::NewState()
{
	ResEditPanelState *p=_NewState();
	p->panel=this;
	return p;
}

BOOL CResEditPanel::_NeedUndo(ResEditPanelState *cur,ResEditPanelState *last)
{
	if (cur->resdata->Equal(*last->resdata))
		return FALSE;
	return TRUE;
}

BOOL CResEditPanel::RepairState(ResEditPanelState *state)
{
	return g_ssGuiLib.pUtilRS->RepairResData(state->resdata);
}


void CResEditPanel::RefreshStateMod(BOOL bSave)
{
	StateToControl(_stateToMod);

	if (bSave)
	{
		ResEditPanelState *stateToSave=_GetStateToSave();
		StateToFile(stateToSave);

		if (_stateBackup)
		{
			if (_SupportUndo())
			if (_NeedUndo(stateToSave,_stateBackup))
			{//只有当ResData真正发生变化
				CModManager *modmgr=_view->GetMgr()->FindModMgr("resource");
				if (modmgr)
				{
					CModRes *mod=new CModRes(this);
					mod->_state=NewState();
					mod->_state->Copy(*_stateBackup);

					modmgr->NewModGroup();
					modmgr->PushBack(mod,FALSE);
				}
			}

			_stateBackup->Copy(*stateToSave);
		}

		if (stateToSave!=_stateToMod)
		{
			if (stateToSave)
				stateToSave->CleanAndDelete();
		}
	}
}


void CResEditPanel::AddCtrl(CResEditCtrl *ctrl)
{
	ctrl->SetPanel(this);
	_ctrls.push_back(ctrl);
	ctrl->EnableCtrl(FALSE);
}

void CResEditPanel::LockControl(CResEditCtrl *ctrl)
{
	_lockedctrl.push_back(ctrl);
}
void CResEditPanel::UnlockControl(CResEditCtrl *ctrl)
{
	int idx;
	VEC_FIND(_lockedctrl,ctrl,idx);
	if (idx!=-1)
		_lockedctrl.erase(_lockedctrl.begin()+idx);
}
BOOL CResEditPanel::IsControlLocked(CResEditCtrl *ctrl)
{
	int idx;
	VEC_FIND(_lockedctrl,ctrl,idx);
	return (idx!=-1);
}

//Update the controls in the panel to reflect the state
BOOL CResEditPanel::StateToControl(ResEditPanelState *state)
{
	for (int i=0;i<_ctrls.size();i++)
		_ctrls[i]->Bind(state,!IsControlLocked(_ctrls[i]));

	if (state)
	{
		if (state->resdata)
		{
			CGuiData_Res*dataRes=(CGuiData_Res*)_view->FindData("resource");
			if (dataRes)
				dataRes->state=state;
		}
	}

	if (_view)
		_view->Invalidate();
	if (_view2)
		_view2->Invalidate();
	return TRUE;
}

BOOL CResEditPanel::StateToFile(ResEditPanelState *state)
{
	if (FALSE==_SaveAnchorData(_anchor,state->resdata))
		return FALSE;
	return TRUE;
}

void CResEditPanel::EnablePanel(BOOL bEnable)
{
	for(int i=0;i<_ctrls.size();i++)
		_ctrls[i]->EnableCtrl(bEnable);
	_bEnable=bEnable;
}

void CResEditPanel::ClearAgent_View()
{
	if (_view)
		_view->ClearAgent(0);
}
void CResEditPanel::ClearAgent_View2()
{
	if (_view2)
		_view2->ClearAgent(0);
}

void CResEditPanel::ClearAgent()
{
	ClearAgent_View();
	ClearAgent_View2();
}

void CResEditPanel::AddAgent(CGuiAgent *agent,DWORD priority)
{
	if (!_view)
		return;

	_view->AddAgent(0,agent,priority);
}
void CResEditPanel::UpdateUI()
{
	IFileSystem * _pFS = g_ssGuiLib.pFS;
	std::string pathResRoot=g_ssGuiLib.pRS->GetPath(Path_Res);

	std::string resPath = _anchor.GetRelativePath();
	std::string namefile =pathResRoot+"\\"+ resPath;

	DWORD attr= _pFS->GetFileAttr(namefile.c_str());

	CWnd *wnd2=NULL;
	if (_view2)
		wnd2=_view2->GetWnd();

	if(attr==File_ReadOnly)
	{
		EnablePanel(FALSE);
	}
	else
	{
		EnablePanel(TRUE);
	}

}


void CResEditPanel::_AddCameraController()
{
	CGuiData_Res * data = (CGuiData_Res*)_view->FindData("resource");
	ICamera *camera = data->cam;
	CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0> *agent;
	AddAgent(agent=new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(camera));
	agent->SetFocusPos(i_math::vector3df());
}

void CResEditPanel::SwitchActive()
{
		
}
