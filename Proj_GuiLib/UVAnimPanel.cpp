#include "stdh.h"

#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/ITools.h"
#include "RenderSystem/IAnim.h"
#include "RenderSystem/IRenderPort.h"

#include ".\uvanimpanel.h"
#include "log/LogFile.h"
#include "shaderlib/SLDefines.h"
#include "shaderlib/SLFeature.h"
#include "strlib/strlib.h"
#include "FileDialogBase.h"

CUVAnimPanel::CUVAnimPanel(void):
	_meshSelector(Res_Mesh,"UV anim mesh selector"),_mtlSelector(Res_Mtrl,"UV anim material selector")
{
	_anchor.SetResType(ResA_MapCoord);
	_anchor.SetLabel("UV Anim");
	_light=NULL;
}

CUVAnimPanel::~CUVAnimPanel(void)
{
}

BEGIN_MESSAGE_MAP(CUVAnimPanel,CResEditPanel)
	ON_BN_CLICKED(IDC_MESHSELECTOR,OnChooseMesh)
	ON_BN_CLICKED(IDC_MATERIALSELECTOR,OnChooseMtrl)
END_MESSAGE_MAP()

void CUVAnimPanel::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_BONEANIMANCHOR, _anchor);
	DDX_Control(pDX, IDC_ANIMPIECELIST, _listAP);
	DDX_Control(pDX, IDC_ANIMINFO, _animinfo);
	DDX_Control(pDX,IDC_MESHSELECTOR,_meshSelector);
	DDX_Control(pDX,IDC_MATERIALSELECTOR,_mtlSelector);
}

void CUVAnimPanel::OnChooseMesh()
{
	const char *path=FD_BrowseResource(Res_Mesh,_meshSelector.GetRelativePath());
	if (path[0])
		_meshSelector.SetRelativePath(path);
}

void CUVAnimPanel::OnChooseMtrl()
{
	const char *path=FD_BrowseResource(Res_Mtrl,_mtlSelector.GetRelativePath());
	if (path[0])
		_mtlSelector.SetRelativePath(path);	
}

ResEditPanelState * CUVAnimPanel::_NewState()
{
	return (ResEditPanelState *)new Reps_Anim;
}

BOOL CUVAnimPanel::OnInitDialog()
{
	if(FALSE==CResEditPanel::OnInitDialog())
		FALSE;

	// initialize the edit list box.
	_listAP.Initialize();
	_listAP.SetListEditStyle(_T(" &AnimPieces:"),	LBS_XT_DEFAULT);


	//info caption
	_animinfo.SetCaptionColors(0,RGB(128,128,128),0);
	_animinfo.ModifyCaptionStyle(1);
	_animinfo.SetOffice2003Colors(false);

	//Anim Piece Range
	if (TRUE){
		CRect rc1,rc2;
		GET_CONTROL_RECT(this,IDC_STARTTICK,rc1);
		HIDE_CONTROL(this,IDC_STARTTICK);
		GET_CONTROL_RECT(this,IDC_ENDTICK,rc2);
		HIDE_CONTROL(this,IDC_ENDTICK);
		_rangeAP.Create(rc1,rc2,IDC_STARTTICK,IDC_ENDTICK,this);

		HIDE_CONTROL(this,IDC_APBONELIST);
		HIDE_CONTROL(this,IDC_VIEWOPTIONGRP);
	}

	//AnimCtrl bar create here
	if(TRUE)
	{
		CRect  rcBar;
		GET_CONTROL_RECT(this,IDC_ANIMCONTRLBAR,rcBar);
		_animCtrlBar.Create(IDC_ANIMCONTRLBAR,this,rcBar.left,rcBar.top,rcBar.Width(),WS_CHILD|WS_VISIBLE);
	}	

	AddCtrl(dynamic_cast<CResEditCtrl*>(&_animCtrlBar));
	AddCtrl(dynamic_cast<CResEditCtrl*>(&_listAP));
	AddCtrl(dynamic_cast<CResEditCtrl*>(&_rangeAP));

	_meshSelector.ShowWindow(SW_SHOW);
	_mtlSelector.ShowWindow(SW_SHOW);
	CWnd * plabelWnd = GetDlgItem(IDC_VIEWOPTIONGRP);
	if(plabelWnd)
		plabelWnd->ShowWindow(SW_SHOW);

	return TRUE;
}

void CUVAnimPanel::OnResDataChange(ResData *data)
{
	if(!data) return;
	_stateToMod->SetData(data);
}

BOOL CUVAnimPanel::StateToControl(ResEditPanelState *state0)
{
	if (FALSE==CResEditPanel::StateToControl(state0))
		return FALSE;

	Reps_Anim *state=(Reps_Anim *)state0;
	
	if (state&&state->resdata)
	{
		if (state->iSelAP!=-1)
			_animinfo.SetWindowText(fromMBCS(StrLib_GetStr(((AnimData*)(state->resdata))->animpieces[state->iSelAP].name)));
		else
			_animinfo.SetWindowText(_T("<Empty>"));
	}
	return TRUE;
}


void CUVAnimPanel::Init3d()
{
	_light = g_ssGuiLib.pRS->CreateLight();	
}

void CUVAnimPanel::Clear3d()
{
	SAFE_RELEASE(_light);
}

void CUVAnimPanel::Draw(IRenderPort *rp)
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
	
	IMtrl  * mtl = NULL;;
	IMesh  * mesh = NULL;
	if(TRUE){
		std::string pathMesh = _meshSelector.GetRelativePath();
		std::string pathMtrl = _mtlSelector.GetRelativePath();
		mesh = (IMesh *)g_ssGuiLib.pRS->GetMeshMgr()->ObtainRes(pathMesh.c_str());
		mtl = (IMtrl *)g_ssGuiLib.pRS->GetMtrlMgr()->ObtainRes(pathMtrl.c_str());
	}
	
	Key_mapcoord * key = (Key_mapcoord *)player->Calc(tick);

	i_math::matrix43f matUv;
	matUv.addTranslation(key->uvOff.x,key->uvOff.y,0.0f);
	matUv.addRotationXYZ(key->uvRot);
	matUv.addScale(key->uvTiling.x,key->uvTiling.y,1.0f);
	
	DrawMeshArg arg;
	i_math::vector3df eyeDir;
	ICamera * camera = rp->GetCamera();
	camera->GetEyeDir(eyeDir);
	_light->SetDirLight(eyeDir,ColorAlpha(0xffffff,0xff),ColorAlpha(0xffffff,0xff),0);
	render->BindLight(_light);
	render->BindMesh(mesh,arg);
	render->BindMtrl(mtl,0);
	render->AddEP(EP_uvBaseXform,matUv);
	render->AddFeature(FC_uvBaseXform);
	render->Render();

	SAFE_RELEASE(anim);
	SAFE_RELEASE(mesh);
	SAFE_RELEASE(mtl);
	SAFE_RELEASE(player);
}

void CUVAnimPanel::OnSelect()
{
	ClearAgent();
	_AddCameraController();
}



