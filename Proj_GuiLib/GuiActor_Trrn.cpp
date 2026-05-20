/********************************************************************
	created:	2007/2/15   17:35
	filename: 	e:\IxEngine\Proj_GuiLib\WEditorPanel_Trrn.cpp
	author:		cxi
	
	purpose:	Terrain EditorPanel
*********************************************************************/
#include "stdh.h"

#include "resource.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"

#include "GuiActor_Trrn.h"

#include "GuiAgent_TerrainView.h"

#include "GuiData.h"

#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IWorldSystem.h"


#include "TBLTexSetDlg.h"
#include "TBLImageLib.h"

#include "WndBase.h"
#include "CommonCtrlBase.h"

#include "FileDialogBase.h"

#include "GuiAgent_TerrainOp.h"

#include "ximage.h"

#include "ToolBase.h"

BEGIN_MESSAGE_MAP(CGuiPanel_Trrn, CGuiPanel)
	ON_CBN_SELCHANGE(IDC_COMBO_BRUSHTYPE,OnComBoChangeBrush)
	ON_BN_CLICKED(IDC_CHECK_PAINT,OnPaintStateChange)
END_MESSAGE_MAP()


CGuiPanel_Trrn::CGuiPanel_Trrn(CWnd* pParent):CGuiPanel(IDD_EDITPANEL_TRRN, pParent)
{
	m_selBrush=-1;
}
CGuiPanel_Trrn::~CGuiPanel_Trrn()
{
	_bPaint = FALSE;
	_tools.Release();
}

void CGuiPanel_Trrn::DoDataExchange(CDataExchange* pDX)
{
	DDX_Check(pDX,IDC_CHECK_PAINT,_bPaint);
}

void CGuiPanel_Trrn::OnPaintStateChange()
{
	GuiData_Trrn * data = (GuiData_Trrn *)FindData("terrain");
	if(data){
		UpdateData(TRUE);
		//检查工具列表中的工具的个数，如果没有任何的工具 Paint状态不能设置
		int iSel = _comboBrushTypes.GetCurSel();
		if(iSel<0){
			_bPaint = FALSE;
			//撤销Paint状态的设置
			UpdateData(FALSE);
		}
		data->bPaint = _bPaint;
	}
}

void CGuiPanel_Trrn::UpdatePaintState()
{
	GuiData_Trrn * data = (GuiData_Trrn *)FindData("terrain");
	if(data){
		int iSel = _comboBrushTypes.GetCurSel();
		if(iSel<0)
			data->bPaint = FALSE;
		_bPaint = data->bPaint;	
		UpdateData(FALSE);
	}
}

BOOL CGuiPanel_Trrn::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_EDITPANEL_TRRN,pParent);	
}

BOOL CGuiPanel_Trrn::OnInitDialog()
{
	_comboBrushTypes.SubclassDlgItem(IDC_COMBO_BRUSHTYPE,this);
	_tools.InitializeTools(TOOL_TERRAINBRUSH);	
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CGuiPanel_Trrn::_AddBrushUtil()
{
	_comboBrushTypes.ResetContent();
	int n = _tools.GetNumberOfTools();
	for(int i = 0;i<n;i++)
	{
		CToolBase * br = _tools.GetTool(i);
		for(int m = 0;m < br->NumOfModes();m++)
		{
			CToolBase::Mode * brmode = br->GetMode(m);
			int idx = _comboBrushTypes.AddString(fromMBCS(brmode->name.c_str()));
			_comboBrushTypes.SetItemData(idx,DWORD_PTR(brmode));
		}
	}
	int nItems = _comboBrushTypes.GetCount();
	if(m_selBrush>=0&&m_selBrush<nItems)
	{
		_comboBrushTypes.SetCurSel(m_selBrush);
		OnComBoChangeBrush();
	}
}

void CGuiPanel_Trrn::OnComBoChangeBrush()
{
	m_selBrush = _comboBrushTypes.GetCurSel();
	int nItems = _comboBrushTypes.GetCount();
	
	CGeActor * actor = dynamic_cast<CGeActor *>(this);
	CWnd * pParent =  GetDlgItem(IDC_BRPANEL_ANCHOR); 

	for(int i = 0;i<nItems;i++)
	{
		CToolBase::Mode * mode = (CToolBase::Mode *)_comboBrushTypes.GetItemData(i);
		CToolBase * pUtil = mode->owner;
		pUtil->EndParam(mode->mode);
	}
	
	if(m_selBrush>=0)
	{
		CToolBase::Mode * mode = (CToolBase::Mode *)_comboBrushTypes.GetItemData(m_selBrush);
		CToolBase * pUtil = mode->owner;

		pUtil->BeginParam(pParent,mode->mode,actor,0,"perspective",AGENTPRIORITY_STANDARD+100);
	}
}

void CGuiPanel_Trrn::OnDetachView(CGeView *view,DWORD iLevel)
{
	if(m_selBrush>=0)
	{
		CToolBase::Mode * mode = (CToolBase::Mode *)_comboBrushTypes.GetItemData(m_selBrush);
		if (mode)
		{
			assert(mode);
			CToolBase * pUtil = mode->owner;
			if (pUtil)
				pUtil->EndParam(mode->mode);
		}
	}

	CGuiPanel::OnDetachView(view,iLevel);
}

void CGuiPanel_Trrn::_OccupyActor()
{
	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");
	assert(dataCam);
	CGuiView *view=(CGuiView *)_mgr->FindView("perspective");
	assert(view);
	if (view->GetCurActor()!=static_cast<CGeActor*>(this))
	{
		view->DiscardLevels(1);
		view->AttachActor(0,static_cast<CGeActor*>(this));
		view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]));
		extern void AddGeneralAgents(CGuiView *view);
		AddGeneralAgents(view);
		view->AddAgent(0,new CGuiAgent_TerrainView);
		view->AddAgent(0,new CGuiAgent_TerrainOp());
	}
	
}

void CGuiPanel_Trrn::OnEnterActivity()
{	
	_OccupyActor();
	_AddBrushUtil();
}

void CGuiPanel_Trrn::_StartPaint()
{
	CGuiView *viewMain=(CGuiView *)FindView("perspective");
	if (!viewMain)
		return;

	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");
	if (!dataCam)
		return;

	_OccupyActor();

	viewMain->DiscardLevels(1);//keep 2 levels

	viewMain->AttachActor(0,static_cast<CGeActor*>(this));
	viewMain->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]),10);
	viewMain->Invalidate();
}

ITrrnMapEditor *CGuiPanel_Trrn::GetTrrnMapEditor()
{
	GuiData_Trrn *dataTrrn=(GuiData_Trrn *)_mgr->FindData("terrain");
	return dataTrrn->GetTrrnMapEditor();
}
