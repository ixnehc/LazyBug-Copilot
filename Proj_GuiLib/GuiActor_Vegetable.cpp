#include "stdh.h"

#include "resource.h"

#include "GuiData.h"

#include "WndBase.h"

#include "WorldSystem/IAssetSystemDefines.h"

#include "GuiActor_Vegetable.h"

#include "WorldSystem/IWorldSystem.h"

#include "WorldSystem/IEntitySystem.h"

#include "GObjGrid.h"

#include "GuiAgent_general.h"

#include "GuiData_vegetable.h"

#include "GuiAgent_grassOp.h"

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CGuiPanel_Vegetable,CDialog)

	ON_CONTROL(BN_CLICKED,IDC_CHECK_PAINTVEGETABLE,OnPaintState)
	ON_CONTROL(BN_CLICKED,IDC_CHECK_REMOVEGETABLE2,OnRemoveState)
	
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_RADIUS,OnGrpParamChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_DENSITY,OnGrpParamChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_SCALEMIN,OnGrpParamChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_SCALEMAX,OnGrpParamChange)
	
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_WINDSTRENGTH,OnWndParamChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_WINDSPEED,OnWndParamChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_WINDLEN,OnWndParamChange)
	
END_MESSAGE_MAP()

CGuiPanel_Vegetable::CGuiPanel_Vegetable(CWnd *pParent /* = NULL */)
:CGuiPanel(IDD_ACTOR_VEGETABLE,pParent)
{
}
CGuiPanel_Vegetable::~CGuiPanel_Vegetable(void)
{
}
void CGuiPanel_Vegetable::DoDataExchange(CDataExchange* pDX)
{
	DDX_Check(pDX,IDC_CHECK_PAINTVEGETABLE,_bPaint);
	DDX_Check(pDX,IDC_CHECK_REMOVEGETABLE2,_bRemove);
	DDX_Control(pDX,IDC_EDIT_RADIUS,_editRadius);
	DDX_Control(pDX,IDC_SLIDER_RADIUS,_sliderRadius);
	DDX_Control(pDX,IDC_EDIT_DENSITY,_editDensity);
	DDX_Control(pDX,IDC_SLIDER_DENSITY,_sliderDensity);

	DDX_Control(pDX,IDC_EDIT_SCALEMIN,_editScaleMin);
	DDX_Control(pDX,IDC_SPIN_SCALEMIN,_spinScaleMin);

	DDX_Control(pDX,IDC_EDIT_SCALEMAX,_editScaleMax);
	DDX_Control(pDX,IDC_SPIN_SCALEMAX,_spinScaleMax);

	DDX_Control(pDX,IDC_EDIT_WINDSTRENGTH,_editWindStrength);
	DDX_Control(pDX,IDC_SLIDER_WINDSTRENGTH,_sliderWindStrength);

	DDX_Control(pDX,IDC_EDIT_WINDSPEED,_editWindSpeed);
	DDX_Control(pDX,IDC_SLIDER_WINDSPEED,_sliderWindSpeed);

	DDX_Control(pDX,IDC_EDIT_WINDLEN,_editWindLen);
	DDX_Control(pDX,IDC_SLIDER_WINDLEN,_sliderWindLen);

}

void CGuiPanel_Vegetable::OnGrpParamChange(NMHDR * pNotifyStruct,LRESULT * pResult)
{
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	if(data){
		data->radius = _editRadius.GetFVal();
		data->density = _editDensity.GetFVal();
		data->scaleMin = _editScaleMin.GetFVal();
		data->scaleMax = _editScaleMax.GetFVal();
	}
}

void CGuiPanel_Vegetable::OnWndParamChange(NMHDR * pNotifyStruct,LRESULT * pResult)
{
	ISpgEditor * editor = NULL;
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	if(data)
		editor = data->GetEditor();

	if(editor){
		SpgWindCfg cfg;
		cfg.strength = _editWindStrength.GetFVal();
		cfg.speed = _editWindSpeed.GetFVal();
		cfg.waveLen = _editWindLen.GetFVal();
		editor->SetWindCfg(&cfg);
	}
}
void CGuiPanel_Vegetable::OnRemoveState()
{
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	if(data)
	{
		UpdateData(TRUE);
		if(_bRemove)
		{
			data->op = GuiData_Vegetable::Op_Remove;
			_bPaint = FALSE;
			UpdateData(FALSE);
		}
		else
		{
			if(!_bPaint) data->op = GuiData_Vegetable::Op_Idle;
		}
	}	
}
void CGuiPanel_Vegetable::UpdateState()
{
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	if(!data)
		return;

	switch(data->op){
		case GuiData_Vegetable::Op_Idle:
			_bPaint = FALSE;
			_bRemove = FALSE;
			break;
		case GuiData_Vegetable::Op_Paint:
			_bPaint = TRUE;
			_bRemove = FALSE;
			break;
		case GuiData_Vegetable::Op_Remove:
			_bPaint = FALSE;
			_bRemove = TRUE;
			break;
		default:
			break;
	}

	// 由数据状态更新界面的状态
	UpdateData(FALSE);
}
void CGuiPanel_Vegetable::OnPaintState()
{
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	if(data)
	{
		UpdateData(TRUE);
		if(_bPaint)
		{
			data->op = GuiData_Vegetable::Op_Paint;
			_bRemove = FALSE;
			UpdateData(FALSE);
		}
		else
		{
			if(!_bRemove) 
				data->op = GuiData_Vegetable::Op_Idle;
		}
	}
}
void CGuiPanel_Vegetable::UpdateUI()
{
	ISpgEditor * editor = GetEditor();
	if(editor){
		if(_windPtr.IsBreak())
			_windPtr = editor->GetWindCfg();
	}

	if(_windPtr.FetchChange()){
		const SpgWindCfg * cfg = _windPtr.GetObj();
		if(cfg){
			_editWindLen.SetValue(cfg->waveLen);
			_editWindSpeed.SetValue(cfg->speed);
			_editWindStrength.SetValue(cfg->strength);
		}
	}
	
	if(editor){
		CSscSystemWrapper * ssc = NULL;
		GuiData_System * dataSys = (GuiData_System *)FindData("system");
		if(dataSys)
			ssc = dataSys->ssc;
		
		IBrushLib * pLib = editor->GetSpgLib();
		_libWnd.Bind(pLib);
		_libWnd.SetSsc(ssc);
	}

}

void CGuiPanel_Vegetable::OnDetachView(CGeView *view,DWORD iLevel)
{
	CGuiPanel::OnDetachView(view,iLevel);
}

void CGuiPanel_Vegetable::OnEnterActivity()
{
	CGuiView * view =(CGuiView *)FindView("perspective");
	if(!view)
		return;

	GuiData_Camera * cameradata = (GuiData_Camera *)FindData("cameras");
	if(!cameradata)
		return;
	
	view->DiscardLevels(1);
	view->AttachActor(0,dynamic_cast<CGeActor *>(this));
	view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,FALSE,DRAG_BUTTON_RIGHT,FALSE>(cameradata->cams[Camera_Perspective]),AGENTPRIORITY_STANDARD);
	view->AddAgent(0,new CGuiAgent_GrassOp(),AGENTPRIORITY_STANDARD+1);	
	extern void AddGeneralAgents(CGuiView *view);
	AddGeneralAgents(view);
}

void CGuiPanel_Vegetable::Reset()
{
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	if(data)
	{
		_editRadius.SetValue(data->radius);
		_editDensity.SetValue(data->density);
		
		_editScaleMin.SetValue(data->scaleMin);
		_editScaleMax.SetValue(data->scaleMax);
	}
}

ISpgEditor * CGuiPanel_Vegetable::GetEditor()
{
	ISpgEditor * editor = NULL;

	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	editor = data->GetEditor();

	return editor;
}	

BOOL CGuiPanel_Vegetable::Create(CWnd *pParent)
{
	if(FALSE == CDialog::Create(IDD_ACTOR_VEGETABLE,pParent)) 
		return FALSE;
	return TRUE;
}

BOOL CGuiPanel_Vegetable::OnInitDialog()
{
	if(FALSE == CGuiPanel::OnInitDialog())
		return FALSE;	

	_libWnd.Create(this,IDC_TREE_VEGETABLELIB,
					IDC_STATIC_SPGPORP,IDC_BTN_SCCVEGTABLE,
					"Vegetable Libaray",TRUE);

	_sliderRadius.LinkTo(&_editRadius);
	_editRadius.SetLimits(0.5f,20.0f);

	_sliderDensity.LinkTo(&_editDensity);
	_editDensity.SetLimits(0.0f,50.0f);

	_editScaleMin.SetLimits(0.1f,5.0f);
	_editScaleMin.SetValue(1.0f);
	_spinScaleMin.LinkTo(&_editScaleMin);
	_editScaleMin.SetLimitMax(&_editScaleMax,false);

	_editScaleMax.SetLimits(0.1f,5.0f);
	_editScaleMax.SetValue(1.0f);
	_spinScaleMax.LinkTo(&_editScaleMax);
	_editScaleMax.SetLimitMin(&_editScaleMin,false);

	_sliderWindStrength.LinkTo(&_editWindStrength);
	_editWindStrength.SetLimits(0.0f,1.0f);

	_sliderWindSpeed.LinkTo(&_editWindSpeed);
	_editWindSpeed.SetLimits(0.0f,100.0f);

	_sliderWindLen.LinkTo(&_editWindLen);
	_editWindLen.SetLimits(0.5f,20.0f);

	UpdateData(FALSE);

	return TRUE;
}







