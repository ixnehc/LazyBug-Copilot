
#include "stdh.h"

#include "FileSystem/IFileSystem.h"

#include "GuiActor_NavMesh.h"

#include "GuiAgent_general.h"

#include "GuiData.h"

#include "resource.h"

#include "GuiData_NavMesh.h"

#include "WndBase.h"

#include "MapObjUtil.h"

#include "GuiAgent_NavMeshOp.h"


//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGuiPanel_NavMesh,CGuiPanel)

	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_CELLSIZE,OnParamsChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_CELLHEIGHT,OnParamsChange)

	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_AGENTHEIGHT,OnParamsChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_AGENTRADIUS,OnParamsChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_AGENTMAXCLIMB,OnParamsChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_AGENTMAXSLOPE,OnParamsChange)

	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_MINREGION,OnParamsChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_MERGEREGION,OnParamsChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_MAXEDGELEN,OnParamsChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_MAXEDGEERROR,OnParamsChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_VERPERPOLY,OnParamsChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_SAMPDIST,OnParamsChange)
	ON_NOTIFY(PBN_ONCHANGE,IDC_EDIT_MAXSAMPERROR,OnParamsChange)

	ON_COMMAND(IDC_BUTTON_EXPORTNAVMESH,OnExport)

	ON_COMMAND_RANGE(IDC_RADIO_STARTPOS,IDC_RADIO_SELECTNAVMESH,OnOpChange)
	
END_MESSAGE_MAP()

CGuiPanel_NavMesh::CGuiPanel_NavMesh(CWnd * pParent/* = NULL*/)
:CGuiPanel(IDD_ACTOR_NAVMESH,pParent)
{
}

CGuiPanel_NavMesh::~CGuiPanel_NavMesh()
{
}

void CGuiPanel_NavMesh::DoDataExchange(CDataExchange* pDX)
{
	// Cell
	DDX_Control(pDX,IDC_EDIT_CELLSIZE,_edCellSize);
	DDX_Control(pDX,IDC_SPIN_CELLSIZE,_spCellSize);
	
	DDX_Control(pDX,IDC_EDIT_CELLHEIGHT,_edCellHeight);
	DDX_Control(pDX,IDC_SPIN_CELLHEIGHT,_spCellHeight);

	//Agent
	DDX_Control(pDX,IDC_EDIT_AGENTHEIGHT,_edAgentHeight);
	DDX_Control(pDX,IDC_SPIN_AGENTHEIGHT,_spAgentHeight);

	DDX_Control(pDX,IDC_EDIT_AGENTRADIUS,_edAgentRadius);
	DDX_Control(pDX,IDC_SPIN_AGENTRADIUS,_spAgentRadius);
	
	DDX_Control(pDX,IDC_EDIT_AGENTMAXCLIMB,_edAgentMaxClimb);
	DDX_Control(pDX,IDC_SPIN_AGENTMAXCLIMB,_spAgentMaxClimb);

	DDX_Control(pDX,IDC_EDIT_AGENTMAXSLOPE,_edAgentMaxSlope);
	DDX_Control(pDX,IDC_SPIN_AGENTMAXSLOPE,_spAgentMaxSlope);

	//Region
	DDX_Control(pDX,IDC_EDIT_MINREGION,_edMinRegion);
	DDX_Control(pDX,IDC_SPIN_MINREGION,_spMinRegion);
	
	DDX_Control(pDX,IDC_EDIT_MERGEREGION,_edMergeRegion);
	DDX_Control(pDX,IDC_SPIN_MERGEREGION,_spMergeRegion);

	//Trianglization
	DDX_Control(pDX,IDC_EDIT_MAXEDGELEN,_edMaxEdgeLen);
	DDX_Control(pDX,IDC_SPIN_MAXEDGELEN,_spMaxEdgeLen);

	DDX_Control(pDX,IDC_EDIT_MAXEDGEERROR,_edMaxEdgeError);
	DDX_Control(pDX,IDC_SPIN_MAXEDGEERROR,_spMaxEdgeError);

	DDX_Control(pDX,IDC_EDIT_VERPERPOLY,_edVerPerPoly);
	DDX_Control(pDX,IDC_SPIN_VERPERPOLY,_spVerPerPoly);

	DDX_Control(pDX,IDC_EDIT_SAMPDIST,_edSampleDist);
	DDX_Control(pDX,IDC_SPIN_SAMPDIST,_spSampleDist);

	DDX_Control(pDX,IDC_EDIT_MAXSAMPERROR,_edMaxSampleError);
	DDX_Control(pDX,IDC_SPIN_MAXSAMPERROR,_spMaxSampleError);
}

BOOL CGuiPanel_NavMesh::OnInitDialog()
{
	if(!CGuiPanel::OnInitDialog())
		return FALSE;

	//Cell
	_spCellSize.LinkTo(&_edCellSize);
	_spCellHeight.LinkTo(&_edCellHeight);
	
	_edCellSize.SetLimits(0.01f,1.0f);
	_edCellHeight.SetLimits(0.1f,1.0f);
	
	//Agent
	_spAgentHeight.LinkTo(&_edAgentHeight);
	_spAgentRadius.LinkTo(&_edAgentRadius);
	_spAgentMaxClimb.LinkTo(&_edAgentMaxClimb);
	_spAgentMaxSlope.LinkTo(&_edAgentMaxSlope);

	_edAgentHeight.SetLimits(0.1f,5.0f);
	_edAgentRadius.SetLimits(0,5.0f);
	_edAgentMaxClimb.SetLimits(0.1f,5.0f);
	_edAgentMaxSlope.SetLimits(0,90.0f);

	//Region
	_spMinRegion.LinkTo(&_edMinRegion);
	_spMergeRegion.LinkTo(&_edMergeRegion);

	_edMinRegion.SetLimits(0,150.0f);
	_edMergeRegion.SetLimits(0,150.0f);

	//Trianglization
	_spMaxEdgeLen.LinkTo(&_edMaxEdgeLen);
	_spMaxEdgeError.LinkTo(&_edMaxEdgeError);
	_spVerPerPoly.LinkTo(&_edVerPerPoly);
	_spSampleDist.LinkTo(&_edSampleDist);
	_spMaxSampleError.LinkTo(&_edMaxSampleError);

	_edMaxEdgeLen.SetLimits(0,50.0f);
	_edMaxEdgeError.SetLimits(0.1f,3.0f);
	_edVerPerPoly.SetLimits(3.0f,12.0f);
	_edSampleDist.SetLimits(0,16.0f);
	_edMaxSampleError.SetLimits(0,16.0f);

	return TRUE;
}

void CGuiPanel_NavMesh::_LoadParam()
{
	GuiData_NavMesh * data = (GuiData_NavMesh *)FindData("navmesh");
	if (!data)
		return;

	_edCellSize.SetValue(data->buildParams.cellSize);
	_edCellHeight.SetValue(data->buildParams.cellHeight);

	_edAgentHeight.SetValue(data->buildParams.agentHeight);
	_edAgentRadius.SetValue(data->buildParams.agentRadius);
	_edAgentMaxClimb.SetValue(data->buildParams.agentMaxClimb);
	_edAgentMaxSlope.SetValue(data->buildParams.agentMaxSlope);

	_edMinRegion.SetValue(data->buildParams.regionMinSize);
	_edMergeRegion.SetValue(data->buildParams.regionMergeSize);

	_edMaxEdgeLen.SetValue(data->buildParams.edgeMaxLen);
	_edMaxEdgeError.SetValue(data->buildParams.edgeMaxError);
	_edVerPerPoly.SetValue(data->buildParams.vertsPerPoly);
	_edSampleDist.SetValue(data->buildParams.detailSampleDist);
	_edMaxSampleError.SetValue(data->buildParams.detailSampleMaxError,TRUE);

}

void CGuiPanel_NavMesh::OnOpChange(unsigned int idCtrl)
{
	CButton * bnt = (CButton *)GetDlgItem(idCtrl);

	GuiData_NavMesh * data = (GuiData_NavMesh *)FindData("navmesh");

	if(bnt&&data)
	{
		switch(idCtrl)
		{
		case IDC_RADIO_STARTPOS:
			data->opState = GuiData_NavMesh::OpSetStartPos;
			break;
		case IDC_RADIO_ENDPOS:
			data->opState = GuiData_NavMesh::OpSetEndPos;
			break;
		case IDC_RADIO_BUILDNAVMESH:
			data->opState = GuiData_NavMesh::OpBuildNavMesh;
			break;
		case IDC_RADIO_SELECTNAVMESH:
			data->opState = GuiData_NavMesh::OpSelectNavMesh;
			break;
		default: break;
		}
	}
}

void CGuiPanel_NavMesh::OnExport()
{
	INavMeshEditor *editor = NULL;

	GuiData_NavMesh * data = (GuiData_NavMesh *)FindData("navmesh");
	
	if(data)
		editor = data->GetEditor();
	
	if(editor)
	{
		CFileDialog dlg(FALSE);
		if(IDOK==dlg.DoModal())
		{
			CString name = dlg.GetPathName();
			IFile* fl = g_ssGuiLib.pFS->OpenFileAbs(toMBCS((LPCTSTR)name), FileAccessMode_Write);
			if (fl)
			{
				editor->SaveNavData(fl);
				fl->Close();
			}
		}
	}
}

void CGuiPanel_NavMesh::OnParamsChange(NMHDR * pNotifyStruct,LRESULT *pResult)
{
	GuiData_NavMesh * data = (GuiData_NavMesh *)FindData("navmesh");
	
	if(data)
	{
		data->buildParams.cellSize = _edCellSize.GetFVal();
		data->buildParams.cellHeight = _edCellHeight.GetFVal();
		
		data->buildParams.agentHeight = _edAgentHeight.GetFVal();
		data->buildParams.agentRadius = _edAgentRadius.GetFVal();
		data->buildParams.agentMaxClimb = _edAgentMaxClimb.GetFVal();
		data->buildParams.agentMaxSlope = _edAgentMaxSlope.GetFVal();
		
		data->buildParams.edgeMaxLen = _edMaxEdgeLen.GetFVal();
		data->buildParams.edgeMaxError = _edMaxEdgeError.GetFVal();
		data->buildParams.regionMinSize = _edMinRegion.GetFVal();
		data->buildParams.regionMergeSize = _edMergeRegion.GetFVal();
		data->buildParams.detailSampleDist = _edSampleDist.GetFVal();
		data->buildParams.detailSampleMaxError = _edMaxSampleError.GetFVal();
		data->buildParams.vertsPerPoly = _edVerPerPoly.GetFVal();
		data->buildParams.monotonePartitioning = true;
	}	
}

INavMeshEditor * CGuiPanel_NavMesh::GetEditor()
{
	INavMeshEditor * editor = NULL;
	GuiData_NavMesh * data = (GuiData_NavMesh *)FindData("navmesh");
	if(data)
		editor = data->GetEditor();
	return editor;	
}


BOOL CGuiPanel_NavMesh::Create(CWnd *pParent)
{
	if(FALSE==CDialog::Create(IDD_ACTOR_NAVMESH,pParent))
		return FALSE;
	
	return TRUE;
}

void CGuiPanel_NavMesh::UpdateUI()
{
	INavMeshEditor * editor = NULL;
	GuiData_NavMesh * data = (GuiData_NavMesh *)FindData("navmesh");
	if(data)
		editor = data->GetEditor();

	if(editor){
		editor->SetParams(&(data->buildParams));
	}
}

void CGuiPanel_NavMesh::OnEnterActivity()
{
	CGuiView *view=(CGuiView *)_mgr->FindView("perspective");
	assert(view);

	view->DiscardLevels(1);
	view->AttachActor(0,dynamic_cast<CGeActor *>(this));
	
	view->AddAgent(0,new CGuiAgent_NavMeshOp(),AGENTPRIORITY_STANDARD + 1);

	//加入相机控制 Agent
	GuiData_Camera*dataCam=(GuiData_Camera*)FindData("cameras");
	assert(dataCam);
	view->AddAgent(0,new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataCam->cams[Camera_Perspective]));
	
	extern void AddGeneralAgents(CGuiView *view);
	AddGeneralAgents(view);

	_LoadParam();
}

void CGuiPanel_NavMesh::OnDetachView(CGeView *view,DWORD iLevel)
{
	CGuiPanel::OnDetachView(view,iLevel); //delete inner
}





