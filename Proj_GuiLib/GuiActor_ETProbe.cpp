
#include "stdh.h"

#include "GuiActor_ETProbe.h"

#include "GuiAgent_general.h"

#include "GuiData_ETProbe.h"

#include "GuiAgent_ETProbeCreate.h"

#include "GuiAgent_ETProbeDraw.h"

#include "GuiAgent_ETProbeOp.h"

#include "GuiAgent_ETProbeMove.h"

#include "GuiAgent_ETProbeBake.h"

#include "resource.h"

#include "WndBase.h"

#include "GObjGrid.h"

#include "RichGridIntItem.h"

#include "MapObjUtil.h"

class CETProbeGrid :public CGObjGrid
{
public:
	CETProbeGrid(){_ptr = NULL; }
	void Bind(const ETProbeInfo  *info);
	virtual void OnEndItemChange(CXTPPropertyGridItem *item);
private:
	const ETProbeInfo  * _ptr;
	ETProbeInfo _info;
};
void CETProbeGrid::Bind(const ETProbeInfo  *info)
{
	if(!info){
		_ptr = NULL;
		ResetContent();
		return;
	}

	//没有变化
	GObjBase * src = (const_cast<ETProbeInfo*>(info))->GetGObj();
	if(_ptr!=info||!(_info.GetGObj()->Equals(src))){
		_ptr = info;
		_info.GetGObj()->Copy(src);
		CGObjGrid::Bind(_info.GetGObj());
		ExpandAll();
	}
}

void CETProbeGrid::OnEndItemChange(CXTPPropertyGridItem *item)
{
	IETProbeEditor * editor = NULL;
	CGuiPanel_ETProbe * panel = (CGuiPanel_ETProbe *)GetParent();
	GuiData_ETProbe * data = (GuiData_ETProbe *)panel->FindData("etprobe");
	if(data&&!data->hObjSels.empty())
		editor = data->GetEditor();
	
	if(editor){
		std::vector<HMapObj> hObjsMod;
		hObjsMod.push_back(data->hObjSels.back());
		HMapObj hObjNew = editor->SetETProbe(data->hObjSels.back(),_info);
		if(hObjNew!=data->hObjSels.back())
			hObjsMod.push_back(hObjNew);

		CGeView *view = panel->FindView("perspective");
		CommitMapObjMod(panel->GetModMgr(),view,hObjsMod,editor);
	}

	CGObjGrid::OnEndItemChange(item);
}

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CGuiPanel_ETProbe,CGuiPanel)
	ON_BN_CLICKED(IDC_CHECK_ETPROBECREATE,OnETProbeCreate)
END_MESSAGE_MAP()

CGuiPanel_ETProbe::CGuiPanel_ETProbe(CWnd * pParent/* = NULL*/)
:CGuiPanel(IDD_ACTOR_ETPROBE,pParent)
{
	_bOnCreate = FALSE;
	_pAgentMove = NULL;
	_pGridETProbe = new CETProbeGrid();
}

CGuiPanel_ETProbe::~CGuiPanel_ETProbe()
{
	SAFE_DELETE(_pGridETProbe);
}

void CGuiPanel_ETProbe::OnETProbeCreate()
{
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(data){
		UpdateData(TRUE);
		data->bOnCreate = _bOnCreate;
	}
}

void CGuiPanel_ETProbe::DoDataExchange(CDataExchange* pDX)
{
	DDX_Check(pDX,IDC_CHECK_ETPROBECREATE,_bOnCreate);
}

BOOL CGuiPanel_ETProbe::Create(CWnd *pParent)
{
	if(FALSE==CDialog::Create(IDD_ACTOR_ETPROBE,pParent))
		return FALSE;

	RECT rc;
	GET_CONTROL_RECT(this,IDC_STATIC_ETPROBEPROP,rc);
	_pGridETProbe->Create(rc,this,IDC_STATIC_ETPROBEPROP);

	return TRUE;
}
void CGuiPanel_ETProbe::UpdateUI()
{
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(data){
		 _bOnCreate = data->bOnCreate;
		 UpdateData(FALSE);
	}

	if(_pAgentMove)
		_pAgentMove->UpdateBind();

	//属性数据
	if(_pGridETProbe){
		IETProbeEditor * editor = data->GetEditor();
		if(editor&&!data->hObjSels.empty()){
			const ETProbeInfo * info = editor->GetETProbeInfo(data->hObjSels.back());
			_pGridETProbe->Bind(info);
		}
		else
			_pGridETProbe->Bind(NULL);
	}


}

void CGuiPanel_ETProbe::OnEnterActivity()
{
	CGuiView *view=(CGuiView *)_mgr->FindView("perspective");
	assert(view);

	view->DiscardLevels(1);
	view->AttachActor(0,dynamic_cast<CGeActor *>(this));
	
	//创建
	view->AddAgent(0,new CGuiAgent_ETProbeCreate(),AGENTPRIORITY_STANDARD + 2);
	
	//选择 删除
	view->AddAgent(0,new CGuiAgent_ETProbeOp(),AGENTPRIORITY_STANDARD + 1);
	
	//绘制 缓存节点
	view->AddAgent(0,new CGuiAgent_ETProbeDraw());
	
	//移动
	_pAgentMove = new CGuiAgent_ETProbeMove();
	view->AddAgent(0,_pAgentMove,AGENTPRIORITY_STANDARD + 1);
	
	//烘烤
	view->AddAgent(0,new CGuiAgent_ETProbeBake(),AGENTPRIORITY_STANDARD + 1);

	//加入相机控制 Agent
	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");
	assert(dataCam);
	view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]));
	
	extern void AddGeneralAgents(CGuiView *view);
	AddGeneralAgents(view);
}

void CGuiPanel_ETProbe::OnDetachView(CGeView *view,DWORD iLevel)
{
	CGuiPanel::OnDetachView(view,iLevel); //delete inner
	_pAgentMove = NULL;
}





