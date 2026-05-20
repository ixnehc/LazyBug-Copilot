#include "stdh.h"

#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/IAnim.h"
#include "RenderSystem/ITools.h"
#include "RenderSystem/IMesh.h"

#include ".\colanimpanel.h"
#include "Log/LogFile.h"
#include "shaderlib/SLDefines.h"
#include "shaderlib/SLFeature.h"
#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"

CColAnimPanel::CColAnimPanel(void)
{
	_anchor.SetResType(ResA_MtrlColor);
	_anchor.SetLabel("Color Anim");
}


CColAnimPanel::~CColAnimPanel(void)
{

}

void CColAnimPanel::Draw(IRenderPort *rp)
{
	IAnimPlayer *player = NULL;
	player= g_ssGuiLib.pRS->CreateAnimPlayer();

	IRenderer * render = rp->ObtainRenderer();
	Reps_Anim *state=(Reps_Anim *)_stateToMod;
	extern AnimPiece *GetAP(Reps_Anim *state);
	AnimPiece *ap=GetAP(state);
	
	// no draw
	if(!ap)
		return;

	if(state->bAnimRangeDraging) 	
		_animCtrlBar.Stop();
	else
		_animCtrlBar.Play();

	IAnim *anim=NULL;
	anim=g_ssGuiLib.pRS->GetDynAnimMgr()->Create((AnimData*)(state->resdata));
	if (anim)
		player->Reset(anim,0,0,TRUE);
	
	DWORD tick=ANIMTICK_INFINITE;
	if (state->tCur!=ANIMTICK_INFINITE)
	{
		if(ap) 
			tick=state->tCur-ap->tStart;
		else
			tick=state->tCur;		
	}
	
	Key_col * key = (Key_col *)player->Calc(tick);
	
	
	i_math::vector4df dif;
	dif.x = key->c[0]/255.0f;
	dif.y = key->c[1]/255.0f;
	dif.z = key->c[2]/255.0f;
	dif.w =key->c[3]/255.0f;

	IMtrl  * mtl = NULL;;
	IMesh  * mesh = NULL;
	if(TRUE){
		std::string pathMesh = _meshSelector.GetRelativePath();
		std::string pathMtrl = _mtlSelector.GetRelativePath();
		mesh = (IMesh *)g_ssGuiLib.pRS->GetMeshMgr()->ObtainRes(pathMesh.c_str());
		mtl = (IMtrl *)g_ssGuiLib.pRS->GetMtrlMgr()->ObtainRes(pathMtrl.c_str());
	}

	DrawMeshArg arg;
	i_math::vector3df eyeDir;
	ICamera * camera = rp->GetCamera();
	camera->GetEyeDir(eyeDir);
	_light->SetDirLight(eyeDir,ColorAlpha(0xffffff,0xff),ColorAlpha(0xffffff,0xff),0);
	
	render->BindLight(_light);
	render->BindMesh(mesh,arg);
	render->BindMtrl(mtl,0);
	
	render->AddEP(EP_colDif,dif);
	
	render->AddFeature(FC_col);

    render->Render();	

	SAFE_RELEASE(anim);
	SAFE_RELEASE(mesh);
	SAFE_RELEASE(mtl);
	SAFE_RELEASE(player);
}












