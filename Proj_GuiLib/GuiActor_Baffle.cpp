
#include "stdh.h"

#include "GuiActor_Baffle.h"

#include "GuiData_Baffle.h"

#include "GuiAgent_BaffleCreate.h"

#include "GuiAgent_BaffleDraw.h"

#include "GuiAgent_BaffleMove.h"

#include "GuiAgent_BaffleSelect.h"

#include "GuiAgent_BaffleKeyPtMod.h"

#include "GuiAgent_general.h"

#include "GuiData.h"

#include "resource.h"


BEGIN_MESSAGE_MAP(CGuiPanel_Baffle,CGuiPanel)
	ON_BN_CLICKED(IDC_CHECK_BAFFLECREATE,OnBaffleCreate)
END_MESSAGE_MAP()

CGuiPanel_Baffle::CGuiPanel_Baffle(CWnd * pParent/* = NULL*/)
:CGuiPanel(IDD_ACTOR_BAFFLE,pParent)
{
	_bOnCreate = FALSE;
	_agentMove = NULL;
}

CGuiPanel_Baffle::~CGuiPanel_Baffle()
{
}

void CGuiPanel_Baffle::OnBaffleCreate()
{
	GuiData_Baffle * data = (GuiData_Baffle *)FindData("baffle");
	if(data){
		UpdateData(TRUE);
		data->bOnCreate = _bOnCreate;
		data->bCreateSub = FALSE;
		data->hObjSel = INVALID_HMAPOBJ;
	}
}

void CGuiPanel_Baffle::DoDataExchange(CDataExchange* pDX)
{
	DDX_Check(pDX,IDC_CHECK_BAFFLECREATE,_bOnCreate);
}

BOOL CGuiPanel_Baffle::Create(CWnd *pParent)
{
	if(FALSE==CDialog::Create(IDD_ACTOR_BAFFLE,pParent))
		return FALSE;
	return TRUE;
}

void CGuiPanel_Baffle::UpdateUI()
{
	IBaffleEditor * editor = NULL;
	GuiData_Baffle * data = (GuiData_Baffle *)FindData("baffle");
	if(data){
		 _bOnCreate = data->bOnCreate;
		 UpdateData(FALSE);
		 editor = data->GetEditor();
	}
	if(_agentMove)
		_agentMove->UpdateBind();
}

void CGuiPanel_Baffle::OnEnterActivity()
{
	CGuiView *view=(CGuiView *)_mgr->FindView("perspective");
	assert(view);

	view->DiscardLevels(1);
	view->AttachActor(0,dynamic_cast<CGeActor *>(this));

	//加入创建操作 Agent
	view->AddAgent(0,new CGuiAgent_BaffleCreate(),AGENTPRIORITY_STANDARD+3);
	
	//加入选择编辑 Agent
	_agentMove = new CGuiAgent_BaffleMove(); //只需要移动操作
	view->AddAgent(0,_agentMove,AGENTPRIORITY_STANDARD+1);
	
	//选择 Agent
	view->AddAgent(0,new CGuiAgent_BaffleSelect(),AGENTPRIORITY_STANDARD+1);

	//加入绘制 场景枚举 Agent
	view->AddAgent(0,new CGuiAgent_BaffleDraw(),AGENTPRIORITY_STANDARD);

	//加入 删除 控制点
	view->AddAgent(0,new CGuiAgent_BaffleKeyPtMod(),AGENTPRIORITY_STANDARD + 2);

	//加入相机控制 Agent
	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");
	assert(dataCam);
	view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]));
	
	//加入 静态光照Bake Agent
	view->AddAgent(0,new CGuiAgent_BakeLocal);
}

void CGuiPanel_Baffle::OnDetachView(CGeView *view,DWORD iLevel)
{
	CGuiPanel::OnDetachView(view,iLevel); //delete inner
	_agentMove = NULL;
}





