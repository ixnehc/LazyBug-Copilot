/************************************************************************/
/*
e:\IxEngine\Proj_GuiLib\AnimCtrl.h
author: star
purpose: write a animal control for add to AnimalPanel.
date: 2007-12-26
*/
/************************************************************************/
#include "stdh.h"
#include ".\AxisArrow.h"
#include "spatialtester/spatialtester.h"

#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IRenderPort.h"


CAxisArrow::CAxisArrow(void)
{
	_mesh = NULL;
	_mtrl = NULL;
}

CAxisArrow::~CAxisArrow(void)
{
	Release();
}
void CAxisArrow::Release()
{
	SAFE_RELEASE(_mesh);
	SAFE_RELEASE(_mtrl);
}
BOOL CAxisArrow::Init(IRenderSystem *pRS)
{
	IMeshMgr  *meshMgr=pRS->GetMeshMgr();
	IMtrlMgr  *mtrlMgr=pRS->GetMtrlMgr();

	_mesh=(IMesh *)meshMgr->ObtainRes("_editor\\axisarrow.msh");
	if(!_mesh) return FALSE;
	
	_mtrl=(IMtrl *)mtrlMgr->ObtainRes("_editor\\axisarrow.mtl");
	if(!_mtrl)
	{
		SAFE_RELEASE(_mesh);
		return FALSE;
	}
	return TRUE;
}

void CAxisArrow::Draw(IRenderer * render,i_math::matrix43f * matrix,BOOL bHightLight)
{
	i_math::vector3df hlight(1.0f,1.0f,1.0f);
	i_math::vector3df dim(0.4f,0.4f,0.4f);
	DrawMeshArg drawArg;
	if(!render)  return;
	if(A_Ok==_mesh->Touch()){
		render->BindMats(matrix,1);
		render->BindMesh(_mesh,drawArg);
		render->BindMtrl(_mtrl,0);
		if(bHightLight)
			render->AddEP(EP_colDif,hlight);
		else
			render->AddEP(EP_colDif,dim);
		render->Render();
	}
}

BOOL CAxisArrow::HitTest(i_math::line3df &line ,i_math::matrix43f *mat)
{
	if(!_mesh) return FALSE;
	IMeshSnapshot * meshSnap =  _mesh->ObtainSnapshot();
	SpacialTester spacialTester;
	MeshHitTestResult testResult;

	spacialTester.Set(line);

	MeshSnapshotArg arg;
	meshSnap->TakeSnapshot(*mat,arg);
	extern MeshHitTestResult HitTest(IMeshSnapshot * meshSnap,SpacialTester & spacialTester);
	testResult = HitTest(meshSnap,spacialTester);
	SAFE_RELEASE(meshSnap);

	if(SpacialTester::Intersect == testResult.testResult)
	{
		testResult.Release();
		return TRUE;
	}
	return FALSE;
}

