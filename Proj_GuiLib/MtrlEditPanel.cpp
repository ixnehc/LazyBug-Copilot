/********************************************************************
	created:	2006/10/31   17:20
	filename: 	e:\IxEngine\Proj_GuiLib\MtrlEditPanel.cpp
	author:		cxi
	
	purpose:	Mtrl main edit panel
*********************************************************************/
#include "stdh.h"

#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/ITexture.h"

#include ".\MtrlEditPanel.h"

#include "resdata/ResData.h"
#include "resdata/MtrlData.h"
#include "GuiAgent_general.h"
#include "GuiEditor_res.h"

#include "WndBase.h"

#include "FileDialogBase.h"

#include "stringparser/stringparser.h"

#include "RenderSystem/IRenderSystem.h"

#include <assert.h>

	
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
//Reps_Mtrl
void Reps_Mtrl::CleanAndDelete()
{
	Zero();
	ResEditPanelState::CleanAndDelete();
}

void Reps_Mtrl::Copy(ResEditPanelState &src0)
{
	stateRG=((Reps_Mtrl&)src0).stateRG;
	ResEditPanelState::Copy(src0);
}



//////////////////////////////////////////////////////////////////////////
//CMtrlEditPanel
CMtrlEditPanel::CMtrlEditPanel():
	_sampleanchor(Res_Mesh,"MtrlAnchor_SampleMesh",FALSE)//need not undo/redo
{
	_anchor.SetResType(Res_Mtrl);
	_anchor.SetLabel("MtrlAnchor");

	_mtrl=NULL;
	memset(_lgts,0,sizeof(_lgts));

	_curSample=0;

	_iLod=0;
}


void CMtrlEditPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MTRLANCHOR,_anchor);
	DDX_Control(pDX, IDC_MESHANCHOR_MTRLSAMPLE, _sampleanchor);
}

BEGIN_MESSAGE_MAP(CMtrlEditPanel, CResEditPanel)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_VIEWOPT_MESHCOMBO,OnSampleSelChange)
	ON_BN_CLICKED(IDC_MESHANCHOR_MTRLSAMPLE,OnSampleClick)
	ON_BN_CLICKED(IDC_LOD1,OnLod1)
	ON_BN_CLICKED(IDC_LOD2,OnLod2)
	ON_BN_CLICKED(IDC_LOD3,OnLod3)
	ON_BN_CLICKED(IDC_LOD4,OnLod4)

END_MESSAGE_MAP()

void CMtrlEditPanel::_RecalcLayout()
{
	i_math::recti rc,rc2;
	GetClientRect((CRect*)&rc);


	rc.cutout(1,30,rc2);

	rc.cutout(0,400,rc2);
	rc2.inflate(-4,-4,-4,-4);
	::SetWindowPos(&_gridMtrl,rc2);

	rc.inflate(-4,-4,-4,-4);
	::SetWindowPos(&_vsdlg,rc);
}



BOOL CMtrlEditPanel::OnInitDialog()
{
	CResEditPanel::OnInitDialog();

	extern void LoadMtrlSampleMesh(std::vector<SampleMeshInfo>&samples);
	LoadMtrlSampleMesh(_samples);
	_curSample=0;


	//material grid
	if (TRUE)
	{
		CRect rc(0,0,1,1);
		_gridMtrl.Create(rc,this,IDC_MTRLGRIDSLOT);
		_gridMtrl.SetWindowText(_T("MtrlGrid"));
	}

	_vsdlg.Create(this);
	_vsdlg.SetOwnerName("ResEditor");
	_vsdlg.ShowWindow(SW_SHOW);

	AddCtrl(dynamic_cast<CResEditCtrl*>(&_gridMtrl));

	_gridMtrl.SetHook2(_vsdlg.GetRGHook());

	//the view options
	if (TRUE)
	{
		CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_VIEWOPT_MESHCOMBO);
		assert(pCB);

		for (int i=0;i<_samples.size();i++)
			pCB->AddString(fromMBCS(_samples[i].showname.c_str()));

		pCB->AddString(_T("[User Specified Mesh]"));

		pCB->SetCurSel(_curSample);
		if (_curSample<_samples.size())
			_sampleanchor.EnableWindow(FALSE);
	}

	_iLod=0;
	CHECK_BUTTON(this,IDC_LOD1,TRUE);

	_RecalcLayout();

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CMtrlEditPanel::Init3d()
{
	for (int i=0;i<_samples.size();i++)
		_samples[i].mesh=(IMesh *)g_ssGuiLib.pRS->GetMeshMgr()->ObtainRes(_samples[i].path.c_str());

	_lgts[0]=g_ssGuiLib.pRS->CreateLight();
	_lgts[1]=g_ssGuiLib.pRS->CreateLight();
	_lgts[0]->SetDirLight(i_math::vector3df(1,-1,1).normalize(),ColorAlpha(0x2f2f2f,0xff),ColorAlpha(0xdfdfdf,0xff),ColorAlpha(0xffffff,0xff));
	_lgts[1]->SetDirLight(i_math::vector3df(0,0,-1),ColorAlpha(0xffffff,0xff),ColorAlpha(0xffffff,0xff),ColorAlpha(0xffffff,0xff));
}

void CMtrlEditPanel::Clear3d()
{
	for (int i=0;i<_samples.size();i++)
		SAFE_RELEASE(_samples[i].mesh);

	SAFE_RELEASE(_mtrl);
	for (int i=0;i<ARRAY_SIZE(_lgts);i++)
		SAFE_RELEASE(_lgts[i]);
}

void CMtrlEditPanel::OnResDataChange(ResData *data)
{

	_stateToMod->SetData(data);
}

ResEditPanelState *CMtrlEditPanel::_NewState()
{
	return (ResEditPanelState *)new Reps_Mtrl;
}


//Update the controls in the panel to reflect the state
BOOL CMtrlEditPanel::StateToControl(ResEditPanelState *state0)
{
	if (!CResEditPanel::StateToControl(state0))
		return FALSE;

	return TRUE;
}

#include <fstream>
void CMtrlEditPanel::Draw(IRenderPort *rp)
{
	if(TRUE){
		std::ifstream ifs;
		ifs.open("d:\\info.dat",std::ios_base::in|std::ios_base::binary);
		size_t sz = 0;
		ifs.read((char*)&sz,sizeof(sz));
		std::vector<i_math::vector2df> uvs(sz);
		ifs.read((char *)&(uvs[0]),sz*sizeof(i_math::vector2df));

		size_t nIB = 0;
		ifs.read((char *)&nIB,sizeof(nIB));
		std::vector<WORD> indices(nIB);
		ifs.read((char *)&(indices[0]),nIB*sizeof(WORD));
		ifs.close();

		std::vector<i_math::pos2di> lines;
		WORD * p = &(indices[0]);
		i_math::pos2di uv[3];
		float w = 256.0f;
		for(int i = 0;i<nIB;i+=3){
			for(int k = 0;k<3;++k){
				uv[k].x = int(uvs[p[k]].x*w);
				uv[k].y = int(uvs[p[k]].y*w);				
			}
			lines.push_back(uv[0]);
			lines.push_back(uv[1]);
			lines.push_back(uv[0]);
			lines.push_back(uv[2]);
			lines.push_back(uv[1]);
			lines.push_back(uv[2]);
			p += 3;
		}
		rp->Lines(lines.data(),lines.size()/2,0xffff0000);
	}

	IMesh *mesh=NULL;
	IMtrl *mtrl=NULL;

	ITexture *warp=NULL,*depth=NULL;
	
	if (_curSample<_samples.size())
	{
		mesh=_samples[_curSample].mesh;
		mesh->AddRef();
	}
	else
	{
		if (std::string("")!=_sampleanchor.GetRelativePath())
			mesh=(IMesh *)g_ssGuiLib.pRS->GetMeshMgr()->ObtainRes(_sampleanchor.GetRelativePath());
	}
	if (!mesh)
		goto _final;

	if (_stateToMod->resdata)
		mtrl=g_ssGuiLib.pRS->GetMtrlMgr()->Create((MtrlData*)(_stateToMod->resdata),_anchor.GetRelativePath());
	if (!mtrl)
		goto _final;

	if (mtrl->GetLodCount()<=_iLod)
		goto _final;

	extern BOOL BuildWarpMaps(IRenderPort *rp,IMtrl *mtrl,int iLod,ILight *lgt,ITexture *&warp,ITexture *&depth);
	BuildWarpMaps(rp,mtrl,_iLod,_lgts[0],warp,depth);

	IRenderer *rdr=rp->ObtainRenderer();

	if (rdr)
	{
		extern ShaderCode BuildMtrlShaderCode(IMtrl *mtrl,int iLod,IMesh *mesh,ILight *lgt);
		ShaderCode sc=BuildMtrlShaderCode(mtrl,_iLod,mesh,_lgts[0]);

		IShader *shader=rdr->BeginRaw(sc);
		if (shader)
		{
			extern void BindMtrlEP(IShader *shader,IMtrl *mtrl,int iLod,ILight *lgt,ITexture *warp,ITexture *depth);
			BindMtrlEP(shader,mtrl,_iLod,_lgts[0],warp,depth);

			DrawMeshArg dmg;
			mesh->Draw(shader,*i_math::matrix43f::identity(),dmg);

			rdr->EndRaw(shader);
		}
	}

	//record in _mtrl
	mtrl->AddRef();
	SAFE_RELEASE(_mtrl);
	_mtrl=mtrl;

_final:
	SAFE_RELEASE(mtrl);
	SAFE_RELEASE(mesh);

	SAFE_RELEASE(warp);
	SAFE_RELEASE(depth);
}

void CMtrlEditPanel::OnSize(UINT nType, int cx, int cy)
{
	CResEditPanel::OnSize(nType, cx, cy);

	_RecalcLayout();
}

void CMtrlEditPanel::OnSampleSelChange()
{
	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_VIEWOPT_MESHCOMBO);

	_curSample=pCB->GetCurSel();
	_sampleanchor.EnableWindow(_curSample>=_samples.size());
}

void CMtrlEditPanel::OnSelect()
{
	ClearAgent();
	_AddCameraController();
}

void CMtrlEditPanel::OnSampleClick()
{
	const char *path=FD_BrowseResource(Res_Mesh,_sampleanchor.GetRelativePath());
	if (path[0])
		_sampleanchor.SetRelativePath(path);
}


void CMtrlEditPanel::OnLod1()
{
	_iLod=0;
}
void CMtrlEditPanel::OnLod2()
{
	_iLod=1;
}

void CMtrlEditPanel::OnLod3()
{
	_iLod=2;
}
void CMtrlEditPanel::OnLod4()
{
	_iLod=3;
}

void CMtrlEditPanel::UpdateUI()
{
	CResEditPanel::UpdateUI();
	_vsdlg.UpdateUI();
}
