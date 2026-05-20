
#include "stdh.h"

#include "GuiActor_Baffles.h"

#include "GuiData_Baffles.h"

#include "GuiAgent_BafflesDraw.h"

#include "GuiAgent_BafflesPoints.h"

#include "GuiAgent_general.h"

#include "GuiData.h"

#include "resource.h"


BEGIN_MESSAGE_MAP(CGuiPanel_Baffles,CGuiPanel)
	ON_BN_CLICKED(IDC_CHECK_BAFFLECREATE,OnBaffleCreate)
END_MESSAGE_MAP()

CGuiPanel_Baffles::CGuiPanel_Baffles(CWnd * pParent/* = NULL*/)
:CGuiPanel(IDD_ACTOR_BAFFLE,pParent)
{
	_agentPoints = NULL;
}

CGuiPanel_Baffles::~CGuiPanel_Baffles()
{
}

void CGuiPanel_Baffles::OnBaffleCreate()
{
	GuiData_Baffles * data = (GuiData_Baffles *)FindData("baffle");
	if(data){
		data->hObjSel = INVALID_HMAPOBJ;
		switch(data->stateWork){
			case GuiData_Baffles::Creating:
				data->stateWork = GuiData_Baffles::Idle;
				_agentPoints->EndCreate();
				break;
			case GuiData_Baffles::Idle:
				data->stateWork = GuiData_Baffles::Creating;
				_agentPoints->BeginCreate();
				break;
			default: break;
		}
	}
}

void CGuiPanel_Baffles::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX,IDC_CHECK_BAFFLECREATE,_bntStateCreate);
}

BOOL CGuiPanel_Baffles::Create(CWnd *pParent)
{
	if(FALSE==CDialog::Create(IDD_ACTOR_BAFFLE,pParent))
		return FALSE;
	return TRUE;
}

void CGuiPanel_Baffles::UpdateUI()
{
	IBafflesEditor * editor = NULL;
	GuiData_Baffles * data = (GuiData_Baffles *)FindData("baffle");
	if(data){
		if(data->stateWork==GuiData_Baffles::Creating)
			_bntStateCreate.SetCheck(TRUE);
		else
			_bntStateCreate.SetCheck(FALSE);
		 editor = data->GetEditor();
	}

	if(_agentPoints)
		_agentPoints->UpdateBind();
}

void CGuiPanel_Baffles::OnEnterActivity()
{
	CGuiView *view=(CGuiView *)_mgr->FindView("perspective");
	assert(view);

	view->DiscardLevels(1);
	view->AttachActor(0,dynamic_cast<CGeActor *>(this));

//	//加入选择编辑 Agent
	_agentPoints = new CGuiAgent_BafflesPoints(); //只需要移动操作
	view->AddAgent(0,_agentPoints,AGENTPRIORITY_STANDARD+1);
	
	//加入绘制 场景枚举 Agent
	view->AddAgent(0,new CGuiAgent_BafflesDraw(),AGENTPRIORITY_STANDARD);

	//加入相机控制 Agent
	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");
	assert(dataCam);
	view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]));
	
	extern void AddGeneralAgents(CGuiView *view);
	AddGeneralAgents(view);
}

void CGuiPanel_Baffles::OnDetachView(CGeView *view,DWORD iLevel)
{
	CGuiPanel::OnDetachView(view,iLevel); //delete inner
	_agentPoints = NULL;
}





