
/************************************************************************/
/*
e:\IxEngine\Proj_GuiLib\TransformAnimPanel.h
author: star
purpose: write a animal control for add to AnimalPanel.
date: 2007-11-4
*/
/************************************************************************/
#include "stdh.h"
#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IAnim.h"
#include ".\transformanimpanel.h"
#include "stringparser/stringparser.h"
#include "strlib/strlib.h"
#include "resdata/AnimData.h"
#include "GuiAgent_general.h"
#include "GuiEditor_res.h"
#include "AnimStateDef.h"
#include "RenderSystem/IRenderPort.h"

extern BOOL ProjectScaleMask(IRenderPort *rp,i_math::matrix43f &matTranf);
CTransformAnimPanel::CTransformAnimPanel(void)
{
	_anchor.SetResType(ResA_XForm);
	_anchor.SetLabel("TranformAnchor");
	_mesh=NULL;
	_mtrl=NULL;
	_mtrlEvent=NULL;
	_meshMgr=NULL;
	_mtrlMgr=NULL;
	_newCamer=NULL;
	_light = NULL;
	_player= NULL;
}

CTransformAnimPanel::~CTransformAnimPanel(void)
{

}
void CTransformAnimPanel::OnResDataChange(ResData *data)
{
	_stateToMod->SetData(data);

}

void CTransformAnimPanel::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_BONEANIMANCHOR, _anchor);
	DDX_Control(pDX, IDC_ANIMPIECELIST, _listAP);
	DDX_Control(pDX, IDC_ANIMINFO, _animinfo);
}

BOOL CTransformAnimPanel::OnInitDialog()
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

	CWnd * plabelWnd = GetDlgItem(IDC_VIEWOPTIONGRP);
	if(plabelWnd)
		plabelWnd->ShowWindow(SW_SHOW);

	return TRUE;
}

BOOL CTransformAnimPanel::StateToControl(ResEditPanelState *state0)
{
	if(FALSE==CResEditPanel::StateToControl(state0))
		return FALSE;
	Reps_Anim * state=(Reps_Anim *)state0;
	if(state->resdata)
	{
		if(state->iSelAP!=-1)
			_animinfo.SetWindowText(fromMBCS(StrLib_GetStr(((AnimData*)(state->resdata))->animpieces[state->iSelAP].name)));
		else
			_animinfo.SetWindowText(_T("<Empty>"));

	}
	return TRUE;	
}

ResEditPanelState * CTransformAnimPanel::_NewState(void) 
{
	return (ResEditPanelState *)new Reps_Anim;
}

void CTransformAnimPanel::Init3d()
{
	_player=g_ssGuiLib.pRS->CreateAnimPlayer();
    _meshMgr=g_ssGuiLib.pRS->GetMeshMgr();
	_mesh=(IMesh *)_meshMgr->ObtainRes("_editor\\sphere.msh");
	_mtrlMgr=g_ssGuiLib.pRS->GetMtrlMgr();
	_mtrl=(IMtrl *)_mtrlMgr->ObtainRes("_editor\\spherekey.mtl");
	_mtrlEvent=(IMtrl *)_mtrlMgr->ObtainRes("_editor\\sphereevent.mtl");
	_newCamer=g_ssGuiLib.pRS->CreateCamera();
	_light = g_ssGuiLib.pRS->CreateLight();
	_axisArrow.Init(g_ssGuiLib.pRS);
}
void CTransformAnimPanel::Clear3d()
{
	SAFE_RELEASE(_player);
	SAFE_RELEASE(_mesh);
	SAFE_RELEASE(_mtrl);
	SAFE_RELEASE(_mtrlEvent);
	SAFE_RELEASE(_newCamer);
	SAFE_RELEASE(_light);
	_axisArrow.Release();
}

void CTransformAnimPanel::Draw(IRenderPort *rp)
{
	if(!_player)
		return;

	IAnim * iAnim=NULL;

	Reps_Anim *state=(Reps_Anim *)_stateToMod;
	if(!state||
		!state->resdata||
		state->iSelAP==-1) 
		return;

	AnimData * resData = (AnimData *)(state->resdata);
   AnimPiece * ap= &(resData->animpieces[state->iSelAP]);
 
	AnimThreadParam atParam;
	if(state->resdata)
		iAnim=g_ssGuiLib.pRS->GetDynAnimMgr()->Create((AnimData *)state->resdata);
	
	_player->Reset(iAnim,state->iSelAP,ap->tStart,TRUE);
	
	KeySet *keySet=(KeySet *)resData->GetKeySet();

	//draw the full path
	_drawPathLine(rp,keySet,ColorAlpha(0xffff00,0xff),ap->iStart,ap->iEnd);

	if(TRUE)
	{
		_showEventLabel(rp,ap,_player,keySet,state->tEventAdjust);
		_showKeyframe(rp,ap,keySet,_player);
	}
	
	if(state->bAnimRangeDraging)
		_animCtrlBar.Stop();
	else
		_animCtrlBar.Play();
	
	_showAnim(rp,ap,state->tCur,_player);

	SAFE_RELEASE(iAnim);
}
	
void CTransformAnimPanel::_showEventLabel(IRenderPort *rp,AnimPiece * ap,IAnimPlayer *IAnimPlayer,KeySet * keys,int offset)
{
// 	ICamera *camer=rp->GetCamera();
// 	i_math::vector3df eyeDir;
// 	camer->GetEyeDir(eyeDir);
// 
// 	_light->SetDirLight(eyeDir,0,ColorAlpha(0xffffff,0xff),0);
// 	
// 	for(int i=0;i<ap->events.size();i++) 
// 	{		
// 		int tick= ap->events[i].tEvent+offset;
// 		if(tick<0) tick=0;
// 		//if(aP->events[i].tEvent>aP->tEnd||aP->events[i].tEvent<tick) continue;
// 		Key_xform * key=(Key_xform *)(IAnimPlayer->GetKey(tick));	
// 
// 		i_math::vector3df  pos(0,0,0);
// 		key->v.getMatrix().transformVect(pos,pos);
// 		DrawFontArg  fontArg;
// 		int x,y;
// 		rp->TransPos(pos,x,y);
// 		fontArg.SetLocation(x,y);
// 		rp->DrawText(StrLib_GetStr(aP->events[i].name),fontArg);
// 		
// 		IRenderer * render=rp->ObtainRenderer();
// 		DrawMeshArg drawMeshArg;
// 		if(!_mesh) continue;
// 		//////////////////////////////////////////////////////////////////////////
// 		i_math::matrix43f matTranf;
// 		matTranf=key->v.getMatrix();
// 		ProjectScaleMask(rp,matTranf);
// 
// 		render->BindMesh(_mesh,drawMeshArg);
// 		render->BindMats(&matTranf,1);
// 		if(_mtrlEvent)
// 			render->BindMtrl(_mtrlEvent);
// 		render->BindLight(_light);
// 		render->Render();
// 	}
}
void CTransformAnimPanel::_showKeyframe(IRenderPort *rp,AnimPiece * aP,KeySet * keys,IAnimPlayer *IAnimPlayer)
{
//	Key_xform * key=NULL;
//	ICamera *camer=rp->GetCamera();
//	i_math::vector3df eyeDir;
//	camer->GetEyeDir(eyeDir);
//	_light->SetDirLight(eyeDir,0,ColorAlpha(0xffffff,0xff),0);
//	AnimTick endTick=IAnimPlayer->GetEndTick();
//	AnimTick offset=0;
//	if(aP)
//		offset=aP->tStart;
//
//	for(int i=0;i<keys->GetKeyCount();i++)
//	{
//		if(keys->GetKey(i)->t>aP->tEnd||keys->GetKey(i)->t<aP->tStart) continue;
// 		key=(Key_xform *)IAnimPlayer->GetKey((keys->GetKey(i))->t-offset);
//		i_math::vector3df pos(0,0,0);
//		key->v.getMatrix().transformVect(pos,pos);		
//		IRenderer * render=rp->ObtainRenderer();
//		DrawMeshArg drawMeshArg;
//		if(!_mesh) continue;
//		//////////////////////////////////////////////////////////////////////////	
//		i_math::matrix43f matTranf;
//		matTranf=key->v.getMatrix();
//		ProjectScaleMask(rp,matTranf);
//
//		render->BindMesh(_mesh,drawMeshArg);
//		render->BindMats(&matTranf,1);
//		if(_mtrl)
//		render->BindMtrl(_mtrl);
//		render->BindLight(_light);
//		render->Render();
//	}
}

void CTransformAnimPanel::_drawPathLine(IRenderPort * rp,KeySet * keys,DWORD color,DWORD iStart,DWORD iEnd)
{
	std::vector<i_math::vector3df> lines;
	for(int i = iStart + 1;i<iEnd;++i){
		Key_xform * k0 = (Key_xform *)keys->GetKey(i-1);
		Key_xform * k1 = (Key_xform *)keys->GetKey(i-0);
		lines.push_back(k0->v.pos);
		lines.push_back(k1->v.pos);
	}
	
	if(!lines.empty())
  		rp->Lines(lines.data(),lines.size()/2,color);
}

void CTransformAnimPanel::_showAnim(IRenderPort *rp,AnimPiece * ap,DWORD wTick,IAnimPlayer *IAnimPlayer)
{
	 Key_xform * key=(Key_xform *)_player->Calc(wTick);
	 if(key){
		 i_math::matrix43f  matTrans=  key->v.getMatrix();
		 ICamera *camer=rp->GetCamera();
		 i_math::vector3df  pos(0,0,0),dirEye;
		 camer->GetEyeDir(dirEye);
		 _light->SetDirLight(dirEye,0,0xffffffff,0xff888888);
		 matTrans.transformVect(pos,pos);
// 		 ProjectScaleMask(rp,matTrans);	 
		 IRenderer *render=rp->ObtainRenderer();
		 render->BindLight(_light);
		 _axisArrow.Draw(render,&matTrans);
	 }
}

void CTransformAnimPanel::OnSelect()
{
	ClearAgent();
	_AddCameraController();
}



