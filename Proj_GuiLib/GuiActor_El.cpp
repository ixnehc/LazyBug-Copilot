
#include "stdh.h"

#include "GuiData_El.h"

#include "GuiActor_El.h"

#include "Log/LogDump.h"

#include "timer/profiler.h"

#include "WorldSystem/IEntitySystem.h"

#include "WorldSystem/IAssetSystem.h"

#include "FileSystem/IMapFileDefines.h"

#include "RenderSystem/IRenderSystem.h"

#include "GuiAgent_ProbeOp.h"

#include ".\resource.h"

#include "WMGuiLib.h"

#include "GuiAgent_ProbeDraw.h"

#include "GuiAgent_ProbeMove.h"

#include "GuiAgent_MatrixEdit.h"

//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGuiPanel_El, CGuiPanel)
	ON_WM_DESTROY()
	ON_COMMAND(IDC_CHECK_PSHOWGRID,OnChangeState)
	ON_COMMAND(IDC_CHECK_PSHOWSAMP,OnChangeState)
	ON_COMMAND(IDC_CHECK_ADDPROBE,OnChangeState)

	ON_NOTIFY(PBN_PRECHANGE,IDC_EDIT_PROBELEN,BeginParameterChange)
	ON_NOTIFY(PBN_PRECHANGE,IDC_EDIT_PROBEWIDTH,BeginParameterChange)
	ON_NOTIFY(PBN_PRECHANGE,IDC_EDIT_PROBEHEIGHT,BeginParameterChange)
	ON_NOTIFY(PBN_PRECHANGE,IDC_EDIT_PROBEGRID,BeginParameterChange)

	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_PROBELEN,OnParameterChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_PROBEWIDTH,OnParameterChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_PROBEHEIGHT,OnParameterChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_PROBEGRID,OnParameterChange)

	ON_NOTIFY(PBN_ENDCHANGE,IDC_EDIT_PROBELEN,EndParameterChange)
	ON_NOTIFY(PBN_ENDCHANGE,IDC_EDIT_PROBEWIDTH,EndParameterChange)
	ON_NOTIFY(PBN_ENDCHANGE,IDC_EDIT_PROBEHEIGHT,EndParameterChange)
	ON_NOTIFY(PBN_ENDCHANGE,IDC_EDIT_PROBEGRID,EndParameterChange)


END_MESSAGE_MAP()

CGuiPanel_El::CGuiPanel_El(CWnd* pParent)
:CGuiPanel(IDD_EDITPANEL_EL, pParent)
{
	_pAgentMod = NULL;
	_ver = 0xffffffff;
	_bShowGrid = TRUE;
	_bShowSamp = FALSE;
	_bOnAdd = FALSE;
}

BOOL CGuiPanel_El::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_EDITPANEL_EL,pParent);	
}

void CGuiPanel_El::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX,IDC_EDIT_PROBELEN,_editLen);
	DDX_Control(pDX,IDC_EDIT_PROBEWIDTH,_editWidth);
	DDX_Control(pDX,IDC_EDIT_PROBEHEIGHT,_editHeight);
	DDX_Control(pDX,IDC_EDIT_PROBEGRID,_editGrid);

	DDX_Control(pDX,IDC_SPIN_PROBELEN,_spinLen);
	DDX_Control(pDX,IDC_SPIN_PROBEWIDTH,_spinWidth);
	DDX_Control(pDX,IDC_SPIN_PROBEHEIGHT,_spinHeight);
	DDX_Control(pDX,IDC_SPIN_PROBEGRID,_spinGrid);
	
	DDX_Check(pDX,IDC_CHECK_ADDPROBE,_bOnAdd);
	DDX_Check(pDX,IDC_CHECK_PSHOWGRID,_bShowGrid);
	DDX_Check(pDX,IDC_CHECK_PSHOWSAMP,_bShowSamp);
}
void CGuiPanel_El::OnChangeState()
{
	UpdateData(TRUE);
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(data){
		data->bOnAdd = _bOnAdd;
		data->bShowGrid = _bShowGrid;
		data->bShowSample = _bShowSamp;
	}	
}
void CGuiPanel_El::BeginParameterChange(NMHDR * pNotifyStruct,LRESULT * pResult)
{
	IProbeCubeMapEditor * editor = NULL;
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(data)
		editor = data->GetEditor();
	if(editor&&data->hObjSels.size()>0){
		HMapObj & hObj = data->hObjSels.back();
		editor->GetMapFileBlk(hObj,_ptBlkEdit);
	}
}
void CGuiPanel_El::OnParameterChange(NMHDR * pNotifyStruct,LRESULT * pResult)
{
	_OnChangeParameter();
}
void CGuiPanel_El::EndParameterChange(NMHDR * pNotifyStruct,LRESULT * pResult)
{
	CModManager * mgr = GetModMgr();
	CModBlockBack * mod = NULL;
	IProbeCubeMapEditor * editor = NULL;

	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(data)
		editor = data->GetEditor();

	if(mgr){
		CGeView * pView = FindView("perspective");
		assert(pView);

		mod  = new CModBlockBack(pView);
		assert(data->hObjSels.size()>0);
		HMapObj &hObj = data->hObjSels.back();
		
		i_math::pos2di ptBlk[2];
		if(editor->GetMapFileBlk(hObj,ptBlk[0])){
			if(ptBlk[0]!=_ptBlkEdit){
				ptBlk[1] = _ptBlkEdit;
				mod->BackupBlocks(ptBlk,2);
			}
			else
				mod->BackupBlocks(ptBlk,1);
		}
		else{
			SAFE_DELETE(mod);
		}
	}
	
	editor->Save();
	
	if(mod){
		mod->SetCallBack<CGuiPanel_El>(this,&CGuiPanel_El::OnBackUp,&CGuiPanel_El::OnRestore);	
		Mod_New(mgr,(CModBase *&)(mod));
	}

	UpdateSel(TRUE);
}
void CGuiPanel_El::_OnChangeParameter()
{
	IProbeCubeMapEditor * editor = NULL;
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(data)
		editor = data->GetEditor();

	if(data->hObjSels.size()>0){
		HMapObj & hObj = data->hObjSels.back();
		ProbeCubeInfo info;
		if(editor->GetInfo(hObj,info)){
			i_math::vector3df minEdge = info.aabb.MinEdge;
			i_math::vector3df diag;
			diag.x = _editLen.GetFVal();
			diag.y = _editHeight.GetFVal();
			diag.z = _editWidth.GetFVal();
			info.density = _editGrid.GetFVal();
			info.aabb.MaxEdge = minEdge + diag;
			//更新句柄，修改时可能发生句柄改变
			editor->SetInfo(hObj,info);
		}
	}
}

void CGuiPanel_El::Reset()
{
	EnableWindow(FALSE);
	GuiData_System *dataSys=(GuiData_System*)FindData("system");
	if (!dataSys)
		return;
	EnableWindow(TRUE);
}

//提交变化时保存现场信息
void CGuiPanel_El::OnBackUp(CModBlockBack * mod)
{
	UpdateSel(TRUE);
}
//恢复改变前现场信息
void CGuiPanel_El::OnRestore(CModBlockBack * mod)
{
	UpdateSel(TRUE);
}
void CGuiPanel_El::_OccupyActor()
{
	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");
	assert(dataCam);
	CGuiView *view=(CGuiView *)_mgr->FindView("perspective");
	assert(view);
	if (view->GetCurActor()!=static_cast<CGeActor*>(this))
	{
		view->DiscardLevels(1);
		view->AttachActor(0,static_cast<CGeActor*>(this));

		//一些通用的agent
		view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]));
		view->AddAgent(0,new CGuiAgent_ViewSwitcher);
		view->AddAgent(0,new CGuiAgent_BakeLocal);
		
		view->AddAgent(0,new CGuiAgent_ElDraw,AGENTPRIORITY_STANDARD);
		
		_pAgentMod = new CGuiAgent_ProbeMove(EditMode_Move);

		view->AddAgent(0,_pAgentMod,AGENTPRIORITY_STANDARD + 1);
		view->AddAgent(0,new CGuiAgent_ProbeOp,AGENTPRIORITY_STANDARD);
	}
}
void CGuiPanel_El::UpdateUI()
{
	if(_pAgentMod)
		((CGuiAgent_ProbeMove * )(_pAgentMod))->UpdateBind();
	UpdateSel(FALSE);
}

void CGuiPanel_El::OnEnterActivity()
{
	_OccupyActor();
}
BOOL CGuiPanel_El::OnInitDialog()
{
	if(FALSE==CGuiPanel::OnInitDialog())
		return FALSE;
	
	_spinLen.LinkTo(&_editLen);
	_spinWidth.LinkTo(&_editWidth);
	_spinHeight.LinkTo(&_editHeight);
	_spinGrid.LinkTo(&_editGrid);

	_editLen.SetLimits(LIMIT_LEN_MIN,LIMIT_LEN_MAX);
	_editWidth.SetLimits(LIMIT_LEN_MIN,LIMIT_LEN_MAX);
	_editHeight.SetLimits(LIMIT_LEN_MIN,LIMIT_LEN_MAX);
	_editGrid.SetLimits(0.5f,4.0f);
	
	return TRUE;
}
void CGuiPanel_El::OnDetachView(CGeView *view,DWORD iLevel)
{
	CGuiPanel::OnDetachView(view,iLevel);
	_pAgentMod = NULL;
}

void CGuiPanel_El::UpdateSel(BOOL bForce /*= FALSE*/)  // FALSE:只有节点发生了改变才会去更新界面
{
	IProbeCubeMapEditor* editor = NULL;

	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(data)
		editor = data->GetEditor();
	
	//更新界面状态
	if(data->bOnAdd!=_bOnAdd)
	{
		_bOnAdd = data->bOnAdd;
		UpdateData(FALSE);
	}

    if(!editor)
		return;
	
	HMapObj hObjCur = (data->hObjSels.size()>0)?data->hObjSels.back():INVALID_HMAPOBJ;
	
	//更新选择状态
	if(hObjCur!=_hObjcurSel)
		_hObjcurSel = hObjCur;
	else{
		if(!bForce&&(_ver==data->ver))	
			return; //没有发生改变返回
	}

	if(_ver!=data->ver)
		_ver = data->ver;

	BOOL hasProbe = FALSE;
	if(_hObjcurSel!=INVALID_HMAPOBJ){
		ProbeCubeInfo info;
		if(editor->GetInfo(_hObjcurSel,info)){
			_editLen.Enable(TRUE);
			_editWidth.Enable(TRUE);
			_editHeight.Enable(TRUE);
			_editGrid.Enable(TRUE);
			i_math::vector3df diag = info.aabb.MaxEdge - info.aabb.MinEdge;
			_editLen.SetValue(diag.x);
			_editWidth.SetValue(diag.z);
			_editHeight.SetValue(diag.y);
			_editGrid.SetValue(info.density);
			hasProbe = TRUE;
		}
	}
	
	if(!hasProbe)
	{
		_editLen.Enable(FALSE);
		_editWidth.Enable(FALSE);
		_editHeight.Enable(FALSE);
		_editGrid.Enable(FALSE);
	}
}






