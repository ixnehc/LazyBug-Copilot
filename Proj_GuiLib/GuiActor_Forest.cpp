#include "stdh.h"
#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IGlobalRenv.h"
#include ".\guiactor_forest.h"
#include "resource.h"
#include "GuiData.h"
#include "GuiData_Forest.h"
#include "WndBase.h"
#include "GuiAgent_general.h"

#include "WorldSystem/IAssetSystemDefines.h"
#include "GuiAgent_TreeMove.h"
#include "GuiAgent_treeOperate.h"
#include "GuiAgent_treeadd.h"
#include "WorldSystem/IAssetSystem.h"
#include "BakeLibDlg.h"

//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGuiPanel_Forest,CDialog)
	ON_COMMAND(IDC_BT_BAKESPTLIB,OnBakeSptlib)
	ON_BN_CLICKED(IDC_CHECK_SHOWDUMMIES,UpdateInfo)
	ON_BN_CLICKED(IDC_BTADDTREE,OnSetAddTree)
	ON_NOTIFY(NM_DBLCLK,IDC_TV_LIBTREE,OnTreeDoubleClicked)
	ON_NOTIFY(STN_ONITEMCHANGE,IDC_TV_LIBTREE,OnSptLibChange)
END_MESSAGE_MAP()

void CGuiPanel_Forest::DoDataExchange(CDataExchange* pDX)
{
	DDX_Check(pDX,IDC_CHECK_SHOWDUMMIES,_bShowDummies);
	DDX_Control(pDX,IDC_EDIT_SCALE_MIN,_editScaleMin);
	DDX_Control(pDX,IDC_EDIT_SCALE_MAX,_editScaleMax);
	DDX_Control(pDX,IDC_SPIN_SCALE_MIN,_spinScaleMin);
	DDX_Control(pDX,IDC_SPIN_SCALE_MAX,_spinScaleMax);
}

void CGuiPanel_Forest::OnSptLibChange(NMHDR * pNotifyStruct,LRESULT * pResult)
{
	IForestEditor * editor = NULL;
	GuiData_Forest * data = static_cast<GuiData_Forest *>(FindData("forest"));
	if(data)
		editor = data->GetEditor();
	
	//树的原型库发生改变
	if(editor)
		editor->ReLoadMap();
}

void CGuiPanel_Forest::OnSetAddTree()
{
	GuiData_Forest * data = static_cast<GuiData_Forest *>(FindData("forest"));
	if(data){
		data->bOnAdd = TRUE;
		data->Reseed();
	}
}

void CGuiPanel_Forest::OnTreeDoubleClicked(NMHDR * pNotifyStruct,LRESULT * pResult)
{
	OnSetAddTree();
}

CGuiPanel_Forest::CGuiPanel_Forest(CWnd *pParent/* = NULL*/)
	:CGuiPanel(IDD_ACTOR_FOREST,pParent)
{
	_bShowDummies = FALSE;
	_matAgent = NULL;
}

void CGuiPanel_Forest::UpdateUI()
{	
	GuiData_Forest * data = static_cast<GuiData_Forest *>(FindData("forest"));
	if(!data)
		return;
	
	IForestEditor * editor = data->GetEditor();
	
	_windLibWnd.SetSsc(data->ssc);
	//更新风的列表
	if(editor)
		_windLibWnd.Bind(editor->GetWindLib());
	else
		_windLibWnd.Bind(NULL);

	if(editor){
		//树的原型库
		_sptLibWnd.Bind(editor->GetSptLib());
		_sptLibWnd.SetSsc(data->ssc);
		//更新选中的对象
		data->info.refModel = _sptLibWnd.GetSelUID();
	}
	else{
		_sptLibWnd.Bind(NULL);
		data->info.refModel = INVALID_BRUID;
	}

	if(editor)
	{
		IBrushLib * pLib = editor->GetSptLib();
		if(pLib){
			const IBrush * brush = pLib->Get(data->info.refModel);
			ISpt * pSpt = (ISpt *)pLib->ObtainRes(brush);		
			_viewPanel.SetSpt(pSpt);
			_viewPanel.SetRS(data->pWS->GetRS());
			_viewPanel.Draw();			
		}
		else
			_viewPanel.SetSpt(NULL);
	}

	if(TRUE){
		CButton * pBtCreate = (CButton *)GetDlgItem(IDC_BTADDTREE);
		pBtCreate->SetCheck(data->bOnAdd);
		data->scaleMin = _editScaleMin.GetFVal(); 
		data->scaleMax = _editScaleMax.GetFVal();
	}	

	if(_matAgent)
		_matAgent->UpdateBind();
}

void CGuiPanel_Forest::OnDetachView(CGeView *view,DWORD iLevel)
{
	_matAgent = NULL;
	return CGuiPanel::OnDetachView(view,iLevel);
}

void CGuiPanel_Forest::Reset()
{
}

void CGuiPanel_Forest::OnEnterActivity()
{
	CGuiView * view =(CGuiView *) FindView("perspective");
	if(!view)
		return;

	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");
	if (!dataCam)
		return;
	
	view->DiscardLevels(1);

	view->AttachActor(0,dynamic_cast<CGeActor *>(this));

	view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]),10);

	view->AddAgent(0,new CGuiAgent_treeOperate(),AGENTPRIORITY_STANDARD+1);

	view->AddAgent(0,new CGuiAgent_treeadd(),AGENTPRIORITY_STANDARD+2);

	_matAgent = new CGuiAgent_TreeMove();
	view->AddAgent(0,_matAgent,AGENTPRIORITY_STANDARD+1);

	extern void AddGeneralAgents(CGuiView *view);
	AddGeneralAgents(view);
}

IBrushLib * CGuiPanel_Forest::GetSptLib()
{
	GuiData_Forest * data = static_cast<GuiData_Forest *>(FindData("forest"));
	if(!data||!(data->GetEditor()))
		return NULL;
	
	IForestEditor * editor = data->GetEditor();
	if(editor)
		return editor->GetSptLib();

	return NULL;
}

void CGuiPanel_Forest::UpdateInfo()
{
	UpdateData(TRUE);
	GuiData_Forest * data = (GuiData_Forest *)(FindData("forest"));
	if(data)
		data->colObjVisible = _bShowDummies;
}

BOOL CGuiPanel_Forest::Create(CWnd *pParent)
{
	CDialog::Create(IDD_ACTOR_FOREST,pParent);
	
	RECT rc;
	_sptLibWnd.Create(this,IDC_TV_LIBTREE,IDC_STATIC_SPTPROP,IDC_BT_SSC,"SpeedTree Libaray",FALSE);

	GET_CONTROL_RECT(this,IDC_FSTPANEL_VIEW,rc);
	_viewPanel.Create(NULL,NULL,WS_CHILD|WS_VISIBLE,rc,this,IDC_FSTPANEL_VIEW);

	_windLibWnd.Create(this,IDC_LISTWINDS,IDC_WINDPROPS,IDC_BTN_SSCFOREST,"风的列表");
	
	return TRUE;
}

BOOL CGuiPanel_Forest::OnInitDialog()
{
	if(FALSE == CGuiPanel::OnInitDialog())
		return FALSE;

	_spinScaleMin.LinkTo(&_editScaleMin);
	_spinScaleMax.LinkTo(&_editScaleMax);

	_editScaleMin.SetLimits(0.1f,5.0f);
	_editScaleMax.SetLimits(0.1f,5.0f);

	_editScaleMin.SetValue(0.43f);
	_editScaleMax.SetValue(1.57f);

	_editScaleMin.SetLimitMax(&_editScaleMax,true);
	_editScaleMax.SetLimitMin(&_editScaleMin,false);

	return TRUE;
}

void CGuiPanel_Forest::CommitStatusData(CModBlockBack * mod)
{
	GuiData_Forest * data = (GuiData_Forest *)(FindData("forest"));
	if(data)
		mod->SetAddOnData(&(data->hTreeSels),data->hTreeSels.size()*sizeof(HMapObj));
}

//恢复场景状态
void CGuiPanel_Forest::RestorStatusData(CModBlockBack * mod)
{	
	int size;
	void * statdata=mod->GetAddOnData(size);
	GuiData_Forest * data = (GuiData_Forest *)(FindData("forest"));

	if(data){	
		if(statdata){	
			int n = size/sizeof(HMapObj);
			data->hTreeSels.resize(n);
			data->location.setZero();
			memcpy(&(data->hTreeSels[0]),statdata,size);
		}
		else{
			data->hTreeSels.clear();
		}
	}
}

IForestEditor * CGuiPanel_Forest::GetEditor()
{
	GuiData_Forest * data = (GuiData_Forest *)FindData("forest");
	assert(data);
	return data->GetEditor();
}

void CGuiPanel_Forest::OnBakeSptlib()
{
	GuiData_Forest * data = (GuiData_Forest *)(FindData("forest"));
	if(!data||(!data->GetEditor()))
		return;

	IBrushLib * pSptLib = GetSptLib();
	if(pSptLib){
		CBakeLibDlg dlgBake;
		dlgBake.SetBakeParam(data->ass->GetSS(),pSptLib);
		if(IDOK==dlgBake.DoModal()){
			data->GetEditor()->ReLoadMap();
		}
	}
}




