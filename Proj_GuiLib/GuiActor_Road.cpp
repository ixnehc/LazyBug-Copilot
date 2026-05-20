
#include "stdh.h"

#include "GuiActor_Road.h"

#include "GuiData_Road.h"

#include "GuiAgent_RoadPoints.h"

#include "GuiAgent_general.h"

#include "GuiData.h"

#include "MapObjUtil.h"

#include "resource.h"

void CGuiPanel_Road::CRoadPropGrid::OnEndItemChange(CXTPPropertyGridItem *item)
{
	GuiData_Road * data = (GuiData_Road *)_owner->FindData("road");
	RoadProp * prop = GetData();
	if(!data||!prop)
		return;

	if(data->hObjSel==INVALID_HMAPOBJ)
		data->roadProp = *prop;
	else
	{
		IRoadEditor * editor = data->GetEditor();
		if(editor)
			editor->SetRoadProp(data->hObjSel,*prop);
		CGeView * view = _owner->FindView("perspective");
		CommitMapObjMod(_owner->GetModMgr(),view,data->hObjSel,editor);

		data->roadProp = *prop;
	}

	CLyObjGrid<RoadProp>::OnEndItemChange(item);
}

//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGuiPanel_Road,CGuiPanel)
	ON_BN_CLICKED(ID_CHECK_CREATEROAD,OnRoadCreate)
END_MESSAGE_MAP()

CGuiPanel_Road::CGuiPanel_Road(CWnd * pParent/* = NULL*/)
:CGuiPanel(IDD_ACTOR_ROAD,pParent)
{
	_agentPoints = NULL;
    _agentPaintOpacity = NULL;
}

CGuiPanel_Road::~CGuiPanel_Road()
{
}

void CGuiPanel_Road::OnRoadCreate()
{
	GuiData_Road * data = (GuiData_Road *)FindData("road");
	if(data){
		data->hObjSel = INVALID_HMAPOBJ;
		switch(data->stateWork){
			case GuiData_Road::Creating:
				data->stateWork = GuiData_Road::Idle;
				_agentPoints->EndCreate();
				break;
			case GuiData_Road::Idle:
				data->stateWork = GuiData_Road::Creating;
				_agentPoints->BeginCreate();
				break;
			default: break;
		}
	}
}

void CGuiPanel_Road::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX,ID_CHECK_CREATEROAD,_btnStateCreate);	
}

BOOL CGuiPanel_Road::Create(CWnd *pParent)
{
	if(FALSE==CDialog::Create(IDD_ACTOR_ROAD,pParent))
		return FALSE;

	_roadPropGrid.Create(this,IDC_STATICPTOPS_ROAD);
	_roadPropGrid.SetOwner(this);

	return TRUE;
}

void CGuiPanel_Road::UpdateUI()
{
	IRoadEditor * editor = NULL;	
	RoadProp * prop = NULL;

	GuiData_Road * data = (GuiData_Road *)FindData("road");
	if(data)
	{	
		//设置Button的状态
		if(data->stateWork==GuiData_Road::Creating)
			_btnStateCreate.SetCheck(TRUE);
		else
			_btnStateCreate.SetCheck(FALSE);

		editor = data->GetEditor();
		if(editor)
			prop = editor->GetRoadProp(data->hObjSel);

		if(prop)
		{
			_roadPropGrid.BindData(prop);
			data->roadProp = *prop;
		}
		else
			_roadPropGrid.BindData(&(data->roadProp));
	}

	if(_agentPoints)
		_agentPoints->UpdateBind();
}

void CGuiPanel_Road::OnEnterActivity()
{
	CGuiView *view=(CGuiView *)_mgr->FindView("perspective");
	assert(view);

	view->DiscardLevels(1);
	view->AttachActor(0,dynamic_cast<CGeActor *>(this));

	//加入选择编辑 Agent
	_agentPoints = new CGuiAgent_RoadPoints(); //只需要移动操作
	view->AddAgent(0,_agentPoints,AGENTPRIORITY_STANDARD+1);

    _agentPaintOpacity = new CGuiAgent_PaintRoadOpacity();
    view->AddAgent(0, _agentPaintOpacity, AGENTPRIORITY_STANDARD + 1);

	//加入相机控制 Agent 
	GuiData_Camera *dataCam=(GuiData_Camera*)FindData("cameras");
	assert(dataCam);
	view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]));

	extern void AddGeneralAgents(CGuiView *view);
	AddGeneralAgents(view);
}

void CGuiPanel_Road::OnDetachView(CGeView *view,DWORD iLevel)
{
	CGuiPanel::OnDetachView(view,iLevel); //delete inner
	_agentPoints = NULL;
    _agentPaintOpacity = NULL;
}





