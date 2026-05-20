/********************************************************************
	created:	2006/8/27   17:46
	filename: 	d:\IxEngine\Proj_GuiLib\BoneAnimPanel.cpp
	author:		ixnehc
	
	purpose:	Bone Anim main panel
*********************************************************************/
#include "stdh.h"

#include "RenderSystem/IAnim.h"
#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/IFont.h"


#include ".\BoneAnimPanel.h"

#include "RenderSystem/IRenderSystem.h"

#include "stringparser/stringparser.h"

#include "resdata/ResData.h"
#include "resdata/AnimData.h"

#include "strlib/strlib.h"


#include "WndBase.h"
#include "RenderPortBase.h"

#include "GuiAgent_general.h"
#include "GuiEditor_res.h"

#include "Log/LogFile.h"

#include "timer/profiler.h"

#include "RenderSystem/ITexture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TEST_CMP_TOLERANCE

extern BonesData2::BoneAnimPiece *GetBoneData2AP(Reps_Anim *state);

//////////////////////////////////////////////////////////////////////////
//Reps_Anim
void Reps_Anim::SetData(ResData *data)
{
	ResEditPanelState::SetData(data);
	iSelAP=-1;
	iSelEvent=-1;
}

void Reps_Anim::CleanAndDelete()
{
	Zero();
	ResEditPanelState::CleanAndDelete();
}

void Reps_Anim::Copy(ResEditPanelState &src0)
{
	Reps_Anim &src=(Reps_Anim &)src0;

	iSelAP=src.iSelAP;
	iSelEvent=src.iSelEvent;

	tCur=src.tCur;

	ResEditPanelState::Copy(src0);
}

//return whether any change occurs(if same as the original,return FALSE)
BOOL Reps_Anim::ChangeSelAP(int iSel)
{
	if (iSel==iSelAP)
		return FALSE;
	iSelEvent=0;
	iSelAP=iSel;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CBoneAnimPanel

CBoneAnimPanel::CBoneAnimPanel()
{
	_anchor.SetResType(ResA_Bones2);
	_anchor.SetLabel("BonesAnchor");

	_matrice=NULL;
	_light=NULL;
	_anim = NULL;
	_animStd = NULL;
}


void CBoneAnimPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BONEANIMANCHOR, _anchor);
	DDX_Control(pDX, IDC_ANIMPIECELIST, _listAP);
	DDX_Control(pDX, IDC_ANIMINFO, _animinfo);
}

BEGIN_MESSAGE_MAP(CBoneAnimPanel, CResEditPanel)
END_MESSAGE_MAP()

BOOL CBoneAnimPanel::OnInitDialog()
{
	CResEditPanel::OnInitDialog();

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

		HIDE_CONTROL(this,IDC_VIEWOPTIONGRP);
		HIDE_CONTROL(this,IDC_MESHSELECTOR);
		HIDE_CONTROL(this,IDC_MATERIALSELECTOR);

		_rangeAP.Create(rc1,rc2,IDC_STARTTICK,IDC_ENDTICK,this);
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

	return TRUE;
}

void CBoneAnimPanel::OnResDataChange(ResData *data)
{
	if(!data) return;
	_stateToMod->SetData(data);

	ISkeleton *skl=NULL;
	if (_stateToMod->resdata)
		skl=g_ssGuiLib.pRS->CreateSkeleton(((BonesData2*)(_stateToMod->resdata))->skeleton);

	SAFE_RELEASE(skl);

	_skeletonModel.Create(((BonesData2*)(_stateToMod->resdata))->skeleton,0xffff0000,0xff5ccbff);
}


ResEditPanelState *CBoneAnimPanel::_NewState()
{
	return (ResEditPanelState *)new Reps_Anim;
}

//Update the controls in the panel to reflect the state
BOOL CBoneAnimPanel::StateToControl(ResEditPanelState *state0)
{
	if (FALSE==CResEditPanel::StateToControl(state0))
		return FALSE;

	Reps_Anim *state=(Reps_Anim *)state0;
	if (state->resdata)
	{
		if (state->iSelAP!=-1)
			_animinfo.SetWindowText(fromMBCS(StrLib_GetStr(((BonesData2*)(state->resdata))->animpieces[state->iSelAP].name)));
		else
			_animinfo.SetWindowText(_T("<Empty>"));

		SAFE_RELEASE(_anim);
		_anim = g_ssGuiLib.pRS->GetDynBoneAnimMgr()->Create(state->resdata);

#ifdef TEST_CMP_TOLERANCE		
		_pathAnim = _anchor.GetPath();
		_pathAnimStd = _pathAnim;
		int i = _pathAnimStd.find("ires\\");
		_pathAnimStd = _pathAnimStd.substr(i+5,_pathAnimStd.length());
		RemoveFileSuffix(_pathAnimStd);
		_pathAnimStd.append("#std.ba2");
		SAFE_RELEASE(_animStd);
		_animStd = (IBoneAnim *)g_ssGuiLib.pRS->GetBoneAnimMgr()->ObtainRes(_pathAnimStd.c_str());
		_animStd->ForceTouch();
#endif
	}
	
	return TRUE;
}

void CBoneAnimPanel::Init3d()
{
	_matrice = g_ssGuiLib.pRS->CreateMatrice43();
	_light = g_ssGuiLib.pRS->CreateLight();

}
void CBoneAnimPanel::Clear3d()
{
	SAFE_RELEASE(_matrice);
	SAFE_RELEASE(_light);
	SAFE_RELEASE(_anim);

#ifdef TEST_CMP_TOLERANCE
 	SAFE_RELEASE(_animStd);
#endif

}

void CBoneAnimPanel::Draw(IRenderPort *rp)
{
	//update the current selected anim piece in _animctrl
	Reps_Anim *state=(Reps_Anim *)_stateToMod;

	if (!state->resdata) 
		return;

// 	ICamera *cam=rp->QueryCamera();
// 	cam->SetNearFar(0.1f,1500.0f);

	BonesData2::BoneAnimPiece *ap = GetBoneData2AP(state);

	if (ap)
	{
		if(state->bAnimRangeDraging) 	
			_animCtrlBar.Stop();
		else
			_animCtrlBar.Play();
		//////////////////////////////////////////////////////////////////////////

		DWORD tick=0;
		if (state->tCur!=ANIMTICK_INFINITE)
			tick= state->tCur;
		
		ICamera * camer=rp->GetCamera();
		i_math::vector3df dirEye;
		camer->GetEyeDir(dirEye);
		_light->SetDirLight(dirEye,0,ColorAlpha(0xffffff,0xff),0);
	

		DWORD nBones = _anim->GetSkeleton()->GetBoneCount();
		std::vector<i_math::xformf> xfmKey0(nBones);
		std::vector<i_math::xformf> xfmKey1(nBones);
		std::vector<i_math::matrix43f> mats(nBones);

#ifdef  TEST_CMP_TOLERANCE	
		if(_animStd){
			_animStd->CalcKey(tick,xfmKey1.data(),nBones,state->iSelAP);
			DWORD colorStdAnim = 0xffff0000;
			for(int i = 0;i<nBones;i++)
				xfmKey1[i].getMatrix(mats[i]);
			_DrawSkeleton(rp,mats.data(),&(colorStdAnim));
		}
#endif	
		if(_anim){
			_anim->CalcKey(tick,xfmKey0.data(),nBones,state->iSelAP);
			for(int i = 0;i<nBones;i++)
				xfmKey0[i].getMatrix(mats[i]);
			_DrawSkeleton(rp,mats.data());
		}
		DrawFontArg arg;
		arg.m_ptLoc.set(0,0);
		char buf[255];
		sprintf(buf,"time:%d",tick);
		rp->DrawText(buf,arg);
	}
}

void CBoneAnimPanel::_DrawSkeleton(IRenderPort *rp,i_math::matrix43f * key,DWORD *col/* = NULL*/)
{
	if(key){
		DWORD colorOld = 0;
		if(col){
			colorOld = _skeletonModel.GetColor();
			_skeletonModel.SetColor(*col);
		}
		_skeletonModel.ResetState(CSkeletonModel::State_Normal);
		_skeletonModel.Draw(rp,key);
		if(col)
			_skeletonModel.SetColor(colorOld);
	}
}

void  CBoneAnimPanel::OnSelect()
{
	ClearAgent();
	_AddCameraController();
}





