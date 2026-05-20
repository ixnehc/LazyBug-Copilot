/********************************************************************
	created:	2006/8/23   17:33
	filename: 	e:\IxEngine\Proj_GuiLib\MeshEditPanel.cpp
	author:		cxi
	
	purpose:	mesh main edit panel
*********************************************************************/
#include "stdh.h"

#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/IFont.h"


#include ".\MeshEditPanel.h"

#include "resdata/ResData.h"
#include "resdata/AnimData.h"
#include "resdata/MtrlData.h"
#include "GuiAgent_general.h"
#include "GuiEditor_res.h"

#include "stringparser/stringparser.h"

#include "RenderSystem/IRenderSystem.h"

#include "WndBase.h"



#include <assert.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
//Reps_Mesh
void Reps_Mesh::CleanAndDelete()
{
	Zero();
	ResEditPanelState::CleanAndDelete();
}

void Reps_Mesh::Copy(ResEditPanelState &src0)
{
	ResEditPanelState::Copy(src0);
	iSelFrame=((Reps_Mesh&)src0).iSelFrame;
}

void Reps_Mesh::SetData(ResData *data)
{
	ResEditPanelState::SetData(data);
	iSelFrame=0;
}



//////////////////////////////////////////////////////////////////////////
//CMeshEditPanel
CMeshEditPanel::CMeshEditPanel()
{
	_anchor.SetResType(Res_Mesh);
	_anchor.SetLabel("MeshAnchor");

	_lgt=NULL;
}


void CMeshEditPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MESHANCHOR, _anchor);
}

BEGIN_MESSAGE_MAP(CMeshEditPanel, CResEditPanel)
END_MESSAGE_MAP()

// CMeshEditPanel 消息处理程序

BOOL CMeshEditPanel::OnInitDialog()
{
	if (FALSE==CResEditPanel::OnInitDialog())
		return FALSE;

	//material grid
	if (TRUE)
	{
		CRect rc;
		GET_CONTROL_RECT(this,IDC_MESHCONTENTSLOT,rc);
		HIDE_CONTROL(this,IDC_MESHCONTENTSLOT);
		_meshcontent.Create(CMeshContent::IDD,this);
		_meshcontent.MoveWindow(&rc);
	}

	AddCtrl(dynamic_cast<CResEditCtrl*>(&_meshcontent));

	return TRUE;
}

void CMeshEditPanel::Init3d()
{
	_lgt=g_ssGuiLib.pRS->CreateLight();

	_lgt->SetDirLight(i_math::vector3df(1,-1,1),0,ColorAlpha(0xffffff,0xff),0);
}


void CMeshEditPanel::Clear3d()
{
	SAFE_RELEASE(_lgt);
}

void CMeshEditPanel::OnResDataChange(ResData *data)
{
	_stateToMod->SetData(data);

}

ResEditPanelState *CMeshEditPanel::_NewState()
{
	return (ResEditPanelState *)new Reps_Mesh;
}


//Update the controls in the panel to reflect the state
BOOL CMeshEditPanel::StateToControl(ResEditPanelState *state0)
{
	if (!CResEditPanel::StateToControl(state0))
		return FALSE;

	return TRUE;
}


void CMeshEditPanel::Draw(IRenderPort *rp)
{
	if (!g_ssGuiLib.pRS->GetMeshMgr())
		return;

	IMesh *mesh;
	std::string path;
	path=_anchor.GetRelativePath();
	if (path=="")
		return;
	mesh=(IMesh*)g_ssGuiLib.pRS->GetMeshMgr()->ObtainRes(path.c_str());


	IMtrl *mtrl;
	if (TRUE)
	{
		MtrlData md;
		md.lods.resize(1);
		md.lods[0].state.modeFacing = Facing_Both;
		mtrl=g_ssGuiLib.pRS->GetMtrlMgr()->Create((MtrlData*)&md,"");
		if (!mtrl)
			goto _final;
	}

	Reps_Mesh *state=(Reps_Mesh *)_stateToMod;
	IRenderer *rdr=rp->ObtainRenderer();
	if (rdr)
	{
		i_math::matrix43f mat;
//		mat.setScale(0.02f,0.02f,0.02f);
		rdr->BindMats(&mat,1);
		rdr->BindMtrl(mtrl,0);
		rdr->BindLight(_lgt);
		DrawMeshArg dmg;
		dmg.iLod = state->iLod;
//		dmg.fillmode = D3DFILL_WIREFRAME;
		dmg.SetFrame((float)state->iSelFrame);
		rdr->BindMesh(mesh,dmg);
		rdr->Render();
	}

	if (_stateToMod->resdata)
	{
		std::string s;
		((MeshData*)(_stateToMod->resdata))->CalcContent(s);
		
		DrawFontArg arg;
		arg.SetLocation(10,20);
		rp->DrawText(s.c_str(),arg);
		
		i_math::matrix43f mat;
		MeshData * data = (MeshData *)(_stateToMod->resdata);
		extern BOOL DrawOBB(IRenderPort *rp,i_math::matrix43f &mat,const i_math::aabbox3df &aabb,DWORD col);
		DrawOBB(rp,mat,data->aabb,0xff00ffff);
	}

	if (FALSE)
	if(_stateToMod->resdata){
		std::string s;
		MeshData * data = (MeshData *)(_stateToMod->resdata);
		MeshData::VtxData &vtxs = (data->vtxframes[0]);
		DWORD nVtx = data->vtxframes.m_nVtx;

		std::vector<i_math::vector3df> normals;
		i_math::vector3df p;
		for(int i = 0;i<nVtx;++i){
			p = vtxs.pos[i];
			normals.push_back(p);
			p += vtxs.normal[i];
			normals.push_back(p);
		}
		rp->Lines(normals.data(),normals.size()/2,0xffff00ff);
	}


_final:
	SAFE_RELEASE(mesh);
	SAFE_RELEASE(mtrl);

}
void CMeshEditPanel::OnSelect()
{
	ClearAgent();
	_AddCameraController();
}




