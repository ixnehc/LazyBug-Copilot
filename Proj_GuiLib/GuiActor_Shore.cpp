
#include "stdh.h"

#include "GuiActor_Shore.h"

#include "GuiAgent_general.h"

#include "GuiData.h"

#include "resource.h"

#include "GuiData_Shore.h"

#include "WndBase.h"

#include "MapObjUtil.h"

#include "GuiAgent_ShorePoints.h"

//////////////////////////////////////////////////////////////////////////
void CGuiPanel_Shore::CShoreCPGrid::OnItemChange(CXTPPropertyGridItem *item)
{
	_CommitChange();
}
void CGuiPanel_Shore::CShoreCPGrid::OnEndItemChange(CXTPPropertyGridItem *item)
{
	_CommitChange();

	if(TRUE){
		IShoreEditor * editor = NULL;
		CGuiPanel_Shore * actor = (CGuiPanel_Shore *)GetParent();
		GuiData_Shore * data = (GuiData_Shore *)actor->FindData("shore");
		if(data)
			editor = actor->GetEditor();

		HMapObj hObj = data->hObjSel;
		CGeView * view = actor->FindView("perspective");
		CommitMapObjMod(actor->GetModMgr(),view,hObj,editor);
	}

	CLyObjGrid<CtrlPt_Shore>::OnEndItemChange(item);
}
void CGuiPanel_Shore::CShoreCPGrid::_CommitChange()
{
	IShoreEditor * editor = NULL;
	CGuiPanel_Shore * actor = (CGuiPanel_Shore *)GetParent();
	GuiData_Shore * data = (GuiData_Shore *)actor->FindData("shore");
	if(data)
		editor = actor->GetEditor();
	
	if(editor&&!data->idxSelCPs.empty()){
		DWORD idx = data->idxSelCPs.back();	
		HMapObj hObj = data->hObjSel;
		ICtrlPointPack * pack = editor->GetCtrlPointPack(hObj);
		if(pack){
			ICtrlPointPack * packTemp = editor->NewCtrlPointPack();
			
			packTemp->Clone(pack);
			CtrlPt_Shore * cp = GetData();
			packTemp->At(idx)->Clone(cp);
			editor->SetCtrlPointPack(hObj,packTemp);
			
			packTemp->DeleteMe();
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void CGuiPanel_Shore::CShoreInfoGrid::OnEndItemChange(CXTPPropertyGridItem *item)
{
	CGuiPanel_Shore * actor = (CGuiPanel_Shore *)GetParent();
	GuiData_Shore * data = (GuiData_Shore *)actor->FindData("shore");
	if(data)
		data->info = *GetData();
	CLyObjGrid<ShoreInfo>::OnEndItemChange(item);
}

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CGuiPanel_Shore,CGuiPanel)
	ON_COMMAND(IDC_BUTTON_APPLYSHAPE,OnApplyShape)
	ON_COMMAND(IDC_BUTTON_APPLYBRUSH,OnApplyBrush)
	ON_BN_CLICKED(IDC_CHECK_SHORESHOREWIRE,OnShowWireframe)
END_MESSAGE_MAP()

CGuiPanel_Shore::CGuiPanel_Shore(CWnd * pParent/* = NULL*/)
:CGuiPanel(IDD_ACTOR_SHORE,pParent)
{
	_agentMove = NULL;
	_bWireframe = TRUE;
}

CGuiPanel_Shore::~CGuiPanel_Shore()
{
	SAFE_DELETE(_agentMove);
}

IShoreEditor * CGuiPanel_Shore::GetEditor()
{
	IShoreEditor * editor = NULL;
	GuiData_Shore * data = (GuiData_Shore *)FindData("shore");
	if(data)
		editor = data->GetEditor();
	return editor;	
}

void CGuiPanel_Shore::OnBrChange()
{
	GuiData_Shore * data = (GuiData_Shore *)FindData("shore");
	if(data){
		_shoreWaveLibWnd.SetSelUID(data->brID);
	}
}

void CGuiPanel_Shore::DoDataExchange(CDataExchange* pDX)
{
	DDX_Check(pDX,IDC_CHECK_SHORESHOREWIRE,_bWireframe);
}

BOOL CGuiPanel_Shore::Create(CWnd *pParent)
{
	if(FALSE==CDialog::Create(IDD_ACTOR_SHORE,pParent))
		return FALSE;
	

	_shoreWaveLibWnd.Create(this,IDC_LIST_SHOREBRUSH,IDC_STATIC_SHOREWAVE,IDC_BTN_SSCSHORE,"Shore Wave Brushs");

	_shoreCPGrid.Create(this,IDC_STATIC_SHORECP);
	_shoreCPGrid.SetWindowText(_T("Shore Control Point"));
	_shoreInfoGrid.Create(this,IDC_STATIC_SHOREINFO);

	return TRUE;
}

void CGuiPanel_Shore::OnShowWireframe()
{
	UpdateData(TRUE);
	GuiData_Shore * data = (GuiData_Shore *)FindData("shore");
	if(data)
		data->bShowWireframe = _bWireframe;
}

void CGuiPanel_Shore::OnApplyShape()
{
	IShoreEditor * editor = NULL;
	GuiData_Shore * data = (GuiData_Shore *)FindData("shore");
	if(data)
		editor = data->GetEditor();

	if(editor){
		HMapObj hObj = data->hObjSel;
		const ShoreInfo * info = editor->GetShoreInfo(hObj);
		std::vector<HMapObj> hObjMods;
		if(info){
			HMapObj hObjNew = editor->SetShoreInfo(hObj,data->info);
			data->hObjSel = hObjNew;
			hObjMods.push_back(hObj);
			if(hObj!=hObjNew)
				hObjMods.push_back(hObjNew);
		}
		
		if(!hObjMods.empty()){
			CGeView * view = FindView("perspective");
			CommitMapObjMod(GetModMgr(),view,hObjMods,editor);
		}
	}
}

void CGuiPanel_Shore::OnApplyBrush()
{
	IShoreEditor * editor = NULL;
	GuiData_Shore *data = (GuiData_Shore *)FindData("shore");
	if(data)
		editor = data->GetEditor();

	if(editor){
		HMapObj hObj = data->hObjSel;
		if(editor->AttachBrush(hObj,data->brID)){
			CGeView * view = FindView("perspective");
			CommitMapObjMod(GetModMgr(),view,hObj,editor);
		}
	}
}

void CGuiPanel_Shore::UpdateUI()
{
	IShoreEditor * editor = NULL;
	GuiData_Shore * data = (GuiData_Shore *)FindData("shore");
	if(data)
		editor = data->GetEditor();
	
	if(editor){
		//更新Lib
		_shoreWaveLibWnd.Bind(editor->GetWaveLib());
		data->brID = _shoreWaveLibWnd.GetSelUID();
		
		//Control Point
		CtrlPt_Shore * cp = NULL;
		if(!data->idxSelCPs.empty()){
			HMapObj &hObj =  data->hObjSel;
			ICtrlPointPack * pack = editor->GetCtrlPointPack(hObj);
			if(pack){
				DWORD idx = data->idxSelCPs.back();
				cp = (CtrlPt_Shore *)pack->At(idx);
			}
		}
		_shoreCPGrid.BindData(cp);
		
		//shore info
		_shoreInfoGrid.BindData(&(data->info));
		
		//设置是否显示线框模型
		if(IsActive())
			editor->SetWireframeVisible(data->bShowWireframe);
		else
			editor->SetWireframeVisible(FALSE);
	}
	else{
		_shoreCPGrid.BindData(NULL);
		_shoreWaveLibWnd.Bind(NULL);
	}

	//当线框不存在时 编辑的Agent置为无效
	if(_agentMove)
		_agentMove->UpdateBind();

	CSscSystemWrapper * ssc = NULL;
	GuiData_System * dataSys = (GuiData_System *)FindData("system");
	if(dataSys)
		ssc = dataSys->ssc;
	_shoreWaveLibWnd.SetSsc(ssc);
}

void CGuiPanel_Shore::OnEnterActivity()
{
	CGuiView *view=(CGuiView *)_mgr->FindView("perspective");
	assert(view);

	view->DiscardLevels(1);
	view->AttachActor(0,dynamic_cast<CGeActor *>(this));
	
	//创建Shore
	_agentMove = new CGuiAgent_ShorePoints();
	view->AddAgent(0,_agentMove,AGENTPRIORITY_STANDARD + 2);

	//加入相机控制 Agent
	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");
	assert(dataCam);
	view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]));
	
	extern void AddGeneralAgents(CGuiView *view);
	AddGeneralAgents(view);
}

void CGuiPanel_Shore::OnDetachView(CGeView *view,DWORD iLevel)
{
	CGuiPanel::OnDetachView(view,iLevel); //delete inner
	_agentMove = NULL;
}





