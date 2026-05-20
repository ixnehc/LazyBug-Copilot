
#include "stdh.h"

#include "GuiActor_Ridge.h"

#include "GuiData_Ridge.h"

#include "GuiAgent_RidgeDraw.h"

#include "GuiAgent_RidgePoints.h"

#include "GuiAgent_TerrainView.h"

#include "GuiAgent_general.h"

#include "GuiData.h"

#include "resource.h"


BEGIN_MESSAGE_MAP(CGuiPanel_Ridge,CGuiPanel)
	ON_BN_CLICKED(ID_CHECK_CREATERIDGE,OnRidgeCreate)
END_MESSAGE_MAP()

CGuiPanel_Ridge::CGuiPanel_Ridge(CWnd * pParent/* = NULL*/)
:CGuiPanel(IDD_ACTOR_RIDGE,pParent)
{
	_agentPoints = NULL;
}

CGuiPanel_Ridge::~CGuiPanel_Ridge()
{
}

void CGuiPanel_Ridge::OnRidgeCreate()
{
	GuiData_Ridge * data = (GuiData_Ridge *)FindData("ridge");
	if(data){
		data->hObjSel = INVALID_HMAPOBJ;
		switch(data->stateCreate){
			case GuiData_Ridge::Creating:
				data->stateCreate = GuiData_Ridge::Idle;
				_agentPoints->EndCreate();
				break;
			case GuiData_Ridge::Idle:
				data->stateCreate = GuiData_Ridge::Creating;
				_agentPoints->BeginCreate();
				break;
			default: break;
		}
	}
}

void CGuiPanel_Ridge::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX,ID_CHECK_CREATERIDGE,_btnStateCreate);
}

BOOL CGuiPanel_Ridge::Create(CWnd *pParent)
{
	if(FALSE==CDialog::Create(IDD_ACTOR_RIDGE,pParent))
		return FALSE;
	return TRUE;
}

void CGuiPanel_Ridge::UpdateUI()
{
	IRidgeEditor * editor = NULL;
	GuiData_Ridge * data = (GuiData_Ridge *)FindData("ridge");
	if(data){
		if(data->stateCreate==GuiData_Ridge::Creating)
			_btnStateCreate.SetCheck(TRUE);
		else
			_btnStateCreate.SetCheck(FALSE);
		 editor = data->GetEditor();
	}

	if(_agentPoints)
		_agentPoints->UpdateBind();
}

void CGuiPanel_Ridge::OnEnterActivity()
{
	CGuiView *view=(CGuiView *)_mgr->FindView("perspective");
	assert(view);

	view->DiscardLevels(1);
	view->AttachActor(0,dynamic_cast<CGeActor *>(this));

	//加入选择编辑 Agent
	_agentPoints = new CGuiAgent_RidgePoints(); //只需要移动操作
	view->AddAgent(0,_agentPoints,AGENTPRIORITY_STANDARD+1);
	
	//加入绘制 场景枚举 Agent
	view->AddAgent(0,new CGuiAgent_RidgeDraw(),AGENTPRIORITY_STANDARD);
	
	//加入地表显示方式 Agent
	view->AddAgent(0,new CGuiAgent_TerrainView(),AGENTPRIORITY_STANDARD);
	
	//加入相机控制 Agent
	GuiData_Camera *dataCam=(GuiData_Camera*)FindData("cameras");
	assert(dataCam);
	view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]));
	
	extern void AddGeneralAgents(CGuiView *view);
	AddGeneralAgents(view);
}

void CGuiPanel_Ridge::OnDetachView(CGeView *view,DWORD iLevel)
{
	CGuiPanel::OnDetachView(view,iLevel); //delete inner
	_agentPoints = NULL;
}





