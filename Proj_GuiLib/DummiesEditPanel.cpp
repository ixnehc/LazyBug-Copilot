/************************************************************************/
/*	e:\IxEngine\Proj_GuiLib\dummieseditpanel.h
	purpose: for dummies resource edit support.
	date: 2007-1-7
	author :star
*/
/************************************************************************/
#include "stdh.h"
#include ".\dummieseditpanel.h"
#include "../Interfaces/RenderSystem/IRenderSystem.h"
#include "RenderPortBase.h"
#include "../Common/resdata/DummiesData.h"
#include "WndBase.h"
#include "GuiEditor_res.h"
#include "GuiAgent_general.h"
#include "matrixedit_base.h"



//////////////////////////////////////////////////////////////////////////
extern BOOL GetBoneDefMatrix(i_math::matrix43f &matWorld,SkeletonInfo &skeleton,int idx,i_math::matrix43f * matOffset=NULL);
extern BOOL ProjectScaleMask(IRenderPort *camer,i_math::matrix43f &matTranf);
extern BOOL DrawOBB(IRenderPort *rp,i_math::matrix43f &mat,const i_math::aabbox3df aabb,DWORD col);
extern BOOL DrawOBBModel(IRenderPort *rp,i_math::matrix43f &mat,i_math::vector3df &eyePoint,i_math::aabbox3df aabb,DWORD col);
extern void DrawSphereModel(IRenderPort * rp,float radius,i_math::matrix43f * mat,DWORD col,i_math::vector3df &eyePoint,float nStep=10,int nSeg =20);
//CGuiAgent_DummyEdit

CDummiesEditPanel::CDummiesEditPanel()
{
	_anchor.SetResType(Res_Dummies);
	_anchor.SetLabel("Dummies");
	_pAgentMatrixEdit  = NULL;
	_rp=NULL;
}

CDummiesEditPanel::~CDummiesEditPanel(void)
{

}

void CDummiesEditPanel::Init3d()
{
	_light=g_ssGuiLib.pRS->CreateLight();
	_meshDummy.Init(g_ssGuiLib.pRS);
}
void CDummiesEditPanel::Clear3d()
{
	_meshDummy.Release();
	SAFE_RELEASE(_light);
}
BEGIN_MESSAGE_MAP(CDummiesEditPanel,CResEditPanel)

END_MESSAGE_MAP()

void CDummiesEditPanel::DoDataExchange(CDataExchange* pDX)
{	
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_DUMMYANCHOR,_anchor);
	DDX_Control(pDX,IDC_DUMMIESLIST,_dummiesListWnd);
}

BOOL CDummiesEditPanel::OnInitDialog()
{
	CResEditPanel::OnInitDialog();
	_dummiesListWnd.Initialize();
	_dummiesListWnd.SetListEditStyle(_T("Dummy"),LBS_XT_DEFAULT);
	//add control
	CRect rc;
	GET_CONTROL_RECT(this,IDC_DUMMYPROPERTY,rc);
	_dummyPropertiesWnd.Create(rc,this,IDC_DUMMYPROPERTY);
	AddCtrl(dynamic_cast<CResEditCtrl*>(&_dummiesListWnd));	
	AddCtrl(dynamic_cast<CResEditCtrl *>(&_dummyPropertiesWnd));
	return TRUE;
}

void CDummiesEditPanel::OnResDataChange(ResData *dataNew)
{
	_stateToMod->SetData(dataNew);
	if(!dataNew)  return;
}


void CDummiesEditPanel::Draw(IRenderPort *rp)//should be overidden to draw something in subclass
{	

	if(!rp) return;
	_rp=rp;
	ISkeleton *skeleton=NULL;
	DummiesData *dummies=(DummiesData *)_stateToMod->resdata;
	if(!dummies) return;
	Reps_Dummies * stateDummies=(Reps_Dummies *)_stateToMod;
	IRenderer * render=rp->ObtainRenderer();

	ICamera *camera=rp->GetCamera();
	ASSERT(camera);
	i_math::vector3df dirEye;
	camera->GetEyeDir(dirEye);
	_light->SetDirLight(dirEye,0,ColorAlpha(0xffffff,0xff),0);
	render->BindLight(_light);

	if(_rp)
		_skeletonModel.Create(dummies->skeletonInfo,0xff5ccbff,0xffaaaaaa);
	_skeletonModel.ResetState(CSkeletonModel::State_Normal);
	if(stateDummies->dummyIdx>=0)
		_skeletonModel.SetState(dummies->dummies[stateDummies->dummyIdx].idxBone,CSkeletonModel::State_Selected);
	_skeletonModel.Draw(rp);
	i_math::matrix43f  matWorld,matOffset;
	
	//////////////////////////////////////////////////////////////////////////
	i_math::matrix43f matScale;
	for(int i=0;i<dummies->dummies.size();i++)
	{
		i_math::matrix43f matParent;
		GetBoneDefMatrix(matParent,dummies->skeletonInfo,dummies->dummies[i].idxBone);
		matWorld.makeIdentity();
		matWorld =dummies->dummies[i].matOff*matParent; 
		
		DWORD col = (stateDummies->dummyIdx==i)?ColorAlpha(0xffff00,0x88):ColorAlpha(0x555555,0xff);
		
		DWORD btyte = dummies->dummies[i].getBoundType();
		switch(btyte)
		{
		case DummyInfo::BoundType_Sphere:
			{
				i_math::matrix43f matSphere;
				i_math::vector3df eyePoint;

				i_math::spheref * sph = dummies->dummies[i].getSphere();

				matSphere.setTranslation(sph->center);
				matSphere *=matWorld;
				camera->GetEyePos(eyePoint);

				DrawSphereModel(rp,sph->radius,&matSphere,col,eyePoint,12,24);
				break;
			}
		case DummyInfo::BoundType_AABB:
			{	
				DWORD colabb = (stateDummies->dummyIdx==i)?ColorAlpha(0xffff00,0x88):ColorAlpha(0x555555,0xff);
				i_math::vector3df eyePoint;
				camera->GetEyePos(eyePoint);
				DummyInfo & dum = dummies->dummies[i];
				
				i_math::aabbox3df *aabb = dummies->dummies[i].getAAbb();
				assert(aabb);

				DrawOBBModel(rp,matWorld,eyePoint,*aabb,colabb);
				break;
			}
		case DummyInfo::BoundType_Point:
			{

				matScale = matWorld;
				matScale.setScale(0.1f,0.1f,0.1f);
				matScale=matScale*matWorld;
//				ProjectScaleMask(rp,matScale);
				if(stateDummies->dummyIdx==i)
					_meshDummy.Draw(render,&matScale,TRUE);
				else
					_meshDummy.Draw(render,&matScale,FALSE);
				break;
			}
		default:
			break;
		}

	}	
	//////////////////////////////////////////////////////////////////////////
	
}

BOOL CDummiesEditPanel::StateToControl(ResEditPanelState *state)//Update the controls in the panel to reflect the state
{	
	if(!state)
		return TRUE;
	
	Reps_Dummies * stateDummy = (Reps_Dummies * )(state);
	DummiesData * resData = (DummiesData *)(state->resdata);
	if(stateDummy->dummyIdx>=resData->dummies.size()){
		stateDummy->dummyIdx = resData->dummies.size()-1;
	}

	if (!CResEditPanel::StateToControl(state))
		return FALSE;

	_BindStateMatrix();
	
	return TRUE;
}

BOOL CDummiesEditPanel::StateToFile(ResEditPanelState *state)//Save all the need-saving part of the state(generally it's the resdata the panel is editing)
{
	if(!state)
		return TRUE;

	Reps_Dummies *status =  (Reps_Dummies* )state;
	
	int idx = status->dummyIdx;
	
	//更新逆矩阵
	if(idx >=0)	
	{
		DummiesData * dummiesData = (DummiesData*)state->resdata;
		dummiesData->dummies[idx].matOffInv = dummiesData->dummies[idx].matOff;
		dummiesData->dummies[idx].matOffInv.makeInverse();
	}

	if (FALSE==_SaveAnchorData(_anchor,state->resdata))
		return FALSE;
	
	return TRUE;
}

ResEditPanelState *CDummiesEditPanel::_NewState()
{
	 Reps_Dummies * dummiesState =  new Reps_Dummies;
	 dummiesState->arrowMesh = &_meshDummy;
	 return dummiesState;
}

void CDummiesEditPanel::BeginMatrixEdit(i_math::matrix43f *matrix)
{

	Reps_Dummies *state =  (Reps_Dummies* )_stateToMod;
	MatrixEditData dataBind;
	if(!state||state->dummyIdx == -1)	
		dataBind.matrix = NULL;
	else
	{
		DummiesData * dummiesData = (DummiesData*)state->resdata;
		if(!dummiesData)
			return ;

		i_math::matrix43f * matDefs = NULL;
		dummiesData->skeletonInfo.GetDefMatrix(matDefs);
		i_math::matrix43f matParent = matDefs[dummiesData->dummies[state->dummyIdx].idxBone];
		SAFE_DELETE(matDefs);
		dataBind.matrix = &(dummiesData->dummies[state->dummyIdx].matOff);
		dataBind.matParent = matParent;
	}

	_pAgentMatrixEdit->Bind(dataBind);

//	if(!_pAgentMatrixEdit->IsSelected())
//		_pAgentDummiesSelect->Enable(TRUE);
//	else
//		_pAgentDummiesSelect->Enable(FALSE);

}

void CDummiesEditPanel::OnMatrixEdit(i_math::matrix43f *matrix)
{
	//RefreshStateMod();
}

void CDummiesEditPanel::EndMatrixEdit(i_math::matrix43f *matrix)
{
	Reps_Dummies *state =  (Reps_Dummies* )_stateToMod;
	if(!state)
		return;
	RefreshStateMod();
}

void CDummiesEditPanel::OnSelect()
{
	ClearAgent();

	_AddCameraController();
	
	 _pAgentDummiesSelect = new CDummiesSelectAgent;
	AddAgent(_pAgentDummiesSelect,AGENTPRIORITY_STANDARD - 10);

	_pAgentMatrixEdit = new CGuiAgent_MatrixEdit(EditMode_All);
	fastdelegate::FastDelegate1<i_math::matrix43f *> dlgatpre,dlgaton,dlgatend;
	_BindStateMatrix();

	dlgatpre.bind(this,&CDummiesEditPanel::BeginMatrixEdit);
	dlgaton.bind(this,&CDummiesEditPanel::OnMatrixEdit);
	dlgatend.bind(this,&CDummiesEditPanel::EndMatrixEdit);
	
	_pAgentMatrixEdit->SetEventListener(dlgatpre,dlgaton,dlgatend);  
	AddAgent(_pAgentMatrixEdit);
}
void CDummiesEditPanel::_BindStateMatrix()
{
	Reps_Dummies *state =  (Reps_Dummies* )_stateToMod;
	MatrixEditData dataBind;
	if(!state||state->dummyIdx == -1)	
		dataBind.matrix = NULL;
	else
	{
		DummiesData * dummiesData = (DummiesData*)state->resdata;
		if(!dummiesData)
			return ;

		i_math::matrix43f * matDefs = NULL;
		dummiesData->skeletonInfo.GetDefMatrix(matDefs);
		i_math::matrix43f matParent = matDefs[dummiesData->dummies[state->dummyIdx].idxBone];
		SAFE_DELETE(matDefs);
		dataBind.matrix = &(dummiesData->dummies[state->dummyIdx].matOff);
		dataBind.matParent = matParent;
	}
	
	if(_pAgentMatrixEdit)
		_pAgentMatrixEdit->Bind(dataBind);
}



