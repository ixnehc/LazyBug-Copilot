/********************************************************************
created:	2006/11/11   20:14
filename: 	e:\IxEngine\Proj_GuiLib\MeshContent.cpp
author:		cxi

purpose:	a panel to edit mesh content
*********************************************************************/
#include "stdh.h"
#include "MeshContent.h"
#include ".\meshcontent.h"
#include "MeshEditPanel.h"

#include "resdata/MeshData.h"

#include "stringparser/stringparser.h"

#include "UVAtlasMakeDlg.h"

#include "log/LogDump.h"


// CMeshContent dialog

IMPLEMENT_DYNAMIC(CMeshContent, CDialog)
CMeshContent::CMeshContent(CWnd* pParent /*=NULL*/)
	: CDialog(CMeshContent::IDD, pParent)
{
}

CMeshContent::~CMeshContent()
{
}

void CMeshContent::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FRAMECOMBO, _comboFrame);
	DDX_Control(pDX, IDC_TANGENTCHECK, _btnTangentInfo);
	DDX_Control(pDX, IDC_REMOVEFRAME, _btnRemoveFrame);
	DDX_Control(pDX, IDC_MAKEUVATLAS, _btnMakeUVAtlas);
	DDX_Control(pDX,IDC_COMBOLOD,_comboLod);
}


BEGIN_MESSAGE_MAP(CMeshContent, CDialog)
	ON_CBN_SELCHANGE(IDC_FRAMECOMBO, OnCbnSelchangeFramecombo)
	ON_BN_CLICKED(IDC_TANGENTCHECK, OnBnClickedTangentcheck)
	ON_BN_CLICKED(IDC_REMOVEFRAME, OnBnClickedRemoveframe)
	ON_BN_CLICKED(IDC_MAKEUVATLAS, OnBnClickedMakeuvatlas)
	ON_CBN_SELCHANGE(IDC_COMBOLOD,OnLodChange)
END_MESSAGE_MAP()

void CMeshContent::OnLodChange()
{
	Reps_Mesh *state=GetState();
	state->iLod = _comboLod.GetCurSel();

	_Refresh();

	RefreshMod();
}
// CMeshContent message handlers
void CMeshContent::EnableCtrl(BOOL bActive)
{
	if(bActive)
		EnableWindow(TRUE);
	else
		EnableWindow(FALSE);
}
BOOL CMeshContent::OnInitDialog()
{
	CDialog::OnInitDialog();


	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMeshContent::Bind(ResEditPanelState *state,BOOL bUpdateCtrl)
{
	CResEditCtrl::Bind(state,bUpdateCtrl);
	
	if (bUpdateCtrl)
		_Refresh();
	
}

void CMeshContent::OnCbnSelchangeFramecombo()
{
	// TODO: Add your control notification handler code here
	Reps_Mesh *state=GetState();
	state->iSelFrame=_comboFrame.GetCurSel();

	_Refresh();

	RefreshMod();
}

void CMeshContent::OnBnClickedTangentcheck()
{
	if(!_state||!_state->resdata) return;
	Reps_Mesh *state=GetState();
	MeshData *data=GetMeshData();

	if (_btnTangentInfo.GetCheck()==1)
		data->BuildTangentInfo();
	else
		data->RemoveTangentInfo();

	_Refresh();

	RefreshMod();
}

void CMeshContent::OnBnClickedRemoveframe()
{
	if(!_state||!_state->resdata) return;
	Reps_Mesh *state=GetState();
	MeshData *data=GetMeshData();

	if (data->frames.size()>1)
	{
		data->RemoveFrame(state->iSelFrame);
		if (state->iSelFrame>=data->frames.size())
			state->iSelFrame=data->frames.size()-1;
		_Refresh();

		RefreshMod();
	}
}

void CMeshContent::OnBnClickedMakeuvatlas()
{
	if(!_state||!_state->resdata) return;
	Reps_Mesh *state=GetState();
	MeshData *data=GetMeshData();

	if(data->frames.size()>1)
	{
		AfxMessageBox(_T("By now,only 1-frame mesh is supported!"));
		return;
	}

	MeshData *data2=(MeshData *)ResData_Clone(data);
	CUVAtlasMakeDlg dlg;
	dlg.SetWorkingData(data2,g_ssGuiLib.pRS,g_ssGuiLib.pUtilRS);
	if (IDOK==dlg.DoModal())
	{
		if (dlg.IsModified())
		{
			data->Copy(*data2);
			_Refresh();

			RefreshMod();
		}
	}

	ResData_Delete(data2);
}


void CMeshContent::_Refresh()
{
	Reps_Mesh *state=GetState();
	MeshData *data=GetMeshData();
	_comboFrame.ResetContent();
	_comboLod.ResetContent();
	if (data)
	{
		std::string s;

		for (int i=0;i<data->frames.size();i++)
		{
			FormatString(s,"Frame %02d",i+1);
			_comboFrame.AddString(fromMBCS(s.c_str()));
		}
		_comboFrame.SetCurSel(state->iSelFrame);

		data->CalcContent(s);
	
		int n = data->lodInfos.size();
		for(int i = 0;i<n;i++){
			FormatString(s,"Lod %02d",i);
			_comboLod.AddString(fromMBCS(s.c_str()));
		}
		_comboLod.SetCurSel(state->iLod);
	}


	_btnTangentInfo.EnableWindow(TRUE);
	_btnRemoveFrame.EnableWindow(data&&(data->frames.size()>1));
	_btnMakeUVAtlas.EnableWindow(TRUE);

	_btnTangentInfo.SetCheck(data&&data->HasTangentInfo());
//	_btnDoubleSideNormal.SetCheck(data&&data->IsDblSided());
}


BOOL CMeshContent::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message == WM_KEYDOWN )&&((pMsg->wParam == 'Z')||(pMsg->wParam == 'Y')))
		return FALSE;
	return CDialog::PreTranslateMessage(pMsg);

}
