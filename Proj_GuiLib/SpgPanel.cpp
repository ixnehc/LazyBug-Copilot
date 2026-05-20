#include "stdh.h"


#include "RenderSystem/IMtrl.h"
#include "RenderSystem/ISpeedGrass.h"
#include "RenderSystem/IShader.h"
#include "RenderSystem/IPatchTools.h"
#include "RenderSystem/IRenderPort.h"


#include "WorldSystem/IStdRes.h"

#include ".\uvanimpanel.h"

#include "log/LogFile.h"

#include "shaderlib/SLDefines.h"

#include "SpgPanel.h"

#include "resdata/SpgData.h"

#include "fvfex/fvfex_data.h"

CSpgPanel::CSpgPanel(void)
{
	_anchor.SetResType(Res_Spg);
	_anchor.SetLabel("Speed Glass");
	
	_mtl = NULL;
	_pSpg = NULL;
}
CSpgPanel::~CSpgPanel(void)
{
	SAFE_RELEASE(_mtl);
	SAFE_RELEASE(_pSpg);
}

BEGIN_MESSAGE_MAP(CSpgPanel,CResEditPanel)

END_MESSAGE_MAP()

void CSpgPanel::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX,IDC_SPTGLASSANCHOR,_anchor);
}
BOOL CSpgPanel::OnInitDialog()
{
	if(FALSE== CResEditPanel::OnInitDialog()) 
		return FALSE;
	return TRUE;
}

void CSpgPanel::OnResDataChange(ResData *data)
{
	_stateToMod->SetData(data);
	if(!data)
		return;

	SAFE_RELEASE(_pSpg);
	_pSpg = (ISpg *)g_ssGuiLib.pRS->GetDynSpgMgr()->Create((SpgData *)data,_anchor.GetRelativePath());
	assert(_pSpg);
	_pSpg->ForceTouch();
}
BOOL CSpgPanel::StateToControl(ResEditPanelState *state)
{
	if(FALSE==CResEditPanel::StateToControl(state))
		return FALSE;

	return TRUE;
}
void CSpgPanel::Init3d()
{
	_mtl = (IMtrl *) g_ssGuiLib.pRS->GetMtrlMgr()->ObtainRes(StdRes_Mtrl_VegeNoBatch);
}
void CSpgPanel::Clear3d()
{
	SAFE_RELEASE(_mtl);
	SAFE_RELEASE(_pSpg);
}

void CSpgPanel::OnSelect()
{
	ClearAgent();
	_AddCameraController();
}

void CSpgPanel::Draw(IRenderPort *rp)
{
	if(NULL==_pSpg)
		return;

	i_math::vector4df param,param2;
	IRenderer * render = rp->ObtainRenderer();
	VBBindArg arg;
	IShader * shader = render->BeginRaw(_mtl);
	if(shader){
		_mtl->BindEP(shader,0);
		_mtl->BindState(shader,0);
		i_math::vector3df dir(1.0f,-0.5f,0.0f);
		dir.normalize();

		if (TRUE)
		{
			ITexture * tex = _pSpg->GetTex_D();
			shader->SetEP(EP_diffusemap,tex);
			tex = _pSpg->GetTex_N();
			shader->SetEP(EP_normalmap,tex);
			tex = _pSpg->GetTex_S();
			shader->SetEP(EP_specmap,tex);
		}

		shader->SetEP(EPG_fx4_01,param);

		FVFExData fvfData;
		int posHandle = fvfData.Add(FVFEX_XYZW0);
		int norHandle = fvfData.Add(FVFEX_NORMAL0);
		int uvHandle = fvfData.Add(FVFEX_FLAG_TEX0);
		fvfData.Compute();
	
		DWORD nIB = 0;
		const WORD *pIB = _pSpg->GetIndexData(nIB);
		
		void * pVtx = NULL;
		IRenderSystem * pRS = rp->GetRS();
		IPatchBuilder * patch = pRS->GetPatchBuilder();
		if(patch->Begin(fvfData.GetFVF())){
			patch->Append(_pSpg->GetNumberOfVertexs(),nIB,pIB,pVtx);
			fvfData.SetPtr(pVtx);
			for(int i = 0;i<_pSpg->GetNumberOfVertexs();i++){
				fvfData.Set(posHandle,_pSpg->GetVertex(i));
				fvfData.Set(norHandle,_pSpg->GetNormal(i));
				fvfData.Set(uvHandle,_pSpg->GetTexVertex(i));
				fvfData.Next();
			}
		}
		patch->End();

		if(patch->BindPatch(shader,&arg)){	
			ShaderState state;
			state.modeFacing = Facing_Front;
			param2.set(1.0f,dir.x,dir.y,dir.z);
			shader->SetEP(EPG_fx4_02,param2);
			shader->SetState(state);
			shader->DoShadeRaw();	

			state.modeFacing = Facing_Back;
			param2.set(1.0f,-dir.x,-dir.y,-dir.z);
			shader->SetEP(EPG_fx4_02,param2);
			shader->SetState(state);
			shader->DoShadeRaw();
		}

		render->EndRaw(shader);
	}
}






