#include "stdh.h"
#include "resource.h"

#include "WndBase.h"

#include "WorldSystem/IAssetSystemDefines.h"

#include "GuiActor_Water.h"

#include "WorldSystem/IWorldSystem.h"

#include "WorldSystem/IEntitySystem.h"

#include "GuiAgent_general.h"

#include "GuiAgent_WaterPaint.h"

#include "GuiAgent_WaterSelect.h"

#include "GuiData_Water.h"

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CGuiPanel_Water,CDialog)
	ON_COMMAND(IDC_PAINTWATER,OnPaint)
	ON_COMMAND(IDC_CLEARWATER,OnClear)

	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_BRUSHSIZE,OnBrushSzChange)

END_MESSAGE_MAP()

void CGuiPanel_Water::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX,IDC_EDIT_BRUSHSIZE,_editSize);
	DDX_Control(pDX,IDC_SPIN_BRUSHSIZE,_spinSize);
	DDX_Control(pDX,IDC_SLIDER_BRUSHSIZE,_slideSize);
}

//////////////////////////////////////////////////////////////////////////

void CGuiPanel_Water::OnBrushSzChange(NMHDR * pNotifyStruct ,LRESULT * pResult)
{
	GuiData_Water * dataWater = (GuiData_Water *)FindData("water");
	if(!dataWater)
		return;
	dataWater->brsize = _editSize.GetFVal();
}

void CGuiPanel_Water::OnClear()
{
	GuiData_Water * dataWater = (GuiData_Water *)FindData("water");
	if(!dataWater)
		return;
	dataWater->op = WPaintOP_Clear;
}

void CGuiPanel_Water::OnPaint()
{
	GuiData_Water * dataWater = (GuiData_Water *)FindData("water");
	if(!dataWater)
		return;

	dataWater->op = WPaintOP_Add;
}

void CGuiPanel_Water::OnBrushSelChange(const BRUID & uid)
{
	_libWnd.SetSelUID(uid);
}

CGuiPanel_Water::CGuiPanel_Water(CWnd *pParent/* = NULL*/)
	:CGuiPanel(IDD_ACTOR_WATER,pParent)
{
	_verLib = 0;
}

CGuiPanel_Water::~CGuiPanel_Water(void)
{
}

void CGuiPanel_Water::UpdateUI()
{	
	IWaterEditor * editor = NULL;
	GuiData_Water * dataWater = (GuiData_Water *)FindData("water");
	if(dataWater)
		editor = dataWater->GetWaterEditor();
	
	IBrushLib * pLib = NULL;
	if(editor)
		pLib = editor->GetBrushLib();

	_libWnd.Bind(pLib);
	dataWater->br = _libWnd.GetSelUID();

	//更新Button的状态
	{
		CButton * pBnt = NULL;

		pBnt = (CButton *)GetDlgItem(IDC_PAINTWATER);
		pBnt->SetCheck(dataWater->op==WPaintOP_Add);
		
		pBnt = (CButton *)GetDlgItem(IDC_CLEARWATER);
		pBnt->SetCheck(dataWater->op==WPaintOP_Clear);
	}

	//更新地图
	if(pLib&&pLib->GetMemVersion()!=_verLib){
		editor->ReLoadMap();
		_verLib = pLib->GetMemVersion();
	}

	CGuiView * view =(CGuiView *)FindView("perspective");
	if(view)
		view->Invalidate();	

	CSscSystemWrapper * ssc = NULL;
	GuiData_System * dataSys = (GuiData_System *)FindData("system");
	if(dataSys)
		ssc = dataSys->ssc;
	_libWnd.SetSsc(ssc);

}

void CGuiPanel_Water::OnEnterActivity()
{
	// 初始化界面 显示Actor的编辑状态
	GuiData_Camera * dataCam = (GuiData_Camera *)FindData("cameras");
	if(!dataCam)
		return;
	
	CGuiView * view =(CGuiView *)FindView("perspective");

	if(view){
		view->DiscardLevels(1);		
		view->AttachActor(0,dynamic_cast<CGeActor *>(this));
		view->AddAgent(0,new CGuiAgent_WaterPaint(),AGENTPRIORITY_STANDARD + 1);
		view->AddAgent(0,new CGuiAgent_WaterSelect(),AGENTPRIORITY_STANDARD + 1);
		view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]),10);
		extern void AddGeneralAgents(CGuiView *view);
		AddGeneralAgents(view);
	}
}


IWaterEditor * CGuiPanel_Water::_GetEditor()
{
	IWaterEditor * editor = NULL;

	GuiData_Water * data = (GuiData_Water *)FindData("water");
	if(data&&data->pES)
		editor = data->GetWaterEditor();
	
	return editor;
}

BOOL CGuiPanel_Water::Create(CWnd *pParent)
{
	if(FALSE == CDialog::Create(IDD_ACTOR_WATER,pParent)) 
		return FALSE;

	return TRUE;
}

BOOL CGuiPanel_Water::OnInitDialog()
{
	if(FALSE == CGuiPanel::OnInitDialog())
		return FALSE;
	
	_libWnd.Create(this,IDC_WBRUSHLIST,IDC_WATERBRUSHGRID,IDC_BTN_SSCWATER,"水的原型库");
	
	_spinSize.LinkTo(&_editSize);
	_slideSize.LinkTo(&_editSize);
	_editSize.SetLimits(4.0f,625.0f);

	return TRUE;
}




