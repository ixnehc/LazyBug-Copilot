
#include "stdh.h"

#include "GuiAgent_NavMeshOp.h"

#include "RenderSystem/IRenderPort.h"

#include "WorldSystem/ITrrn.h"

#include "WorldSystem/IWorldSystem.h"

#include "WorldSystem/IStdRes.h"

#include "GuiData_NavMesh.h"

#include "GuiData.h"

#include "AgentCmdID.h"

#include "MapObjUtil.h"

#include "WorldSystem/IEntitySystem.h"

#include "WorldSystem/IAssetSystem.h"

#include "PhysicsSystem/IPhysicsSystem.h"

#include "RenderSystem/IFont.h"

#include "timer/profiler.h"

extern BOOL DrawOBB(IRenderPort *rp,i_math::matrix43f &mat,const i_math::aabbox3df &aabb,DWORD col);

//////////////////////////////////////////////////////////////////////////

CGuiAgent_NavMeshOp::CGuiAgent_NavMeshOp(void)
{
}

CGuiAgent_NavMeshOp::~CGuiAgent_NavMeshOp(void)
{ 

}

BOOL CGuiAgent_NavMeshOp::OnLButtonDown(int x,int y,DWORD flag)
{	
	GuiData_NavMesh * data = (GuiData_NavMesh *)FindData("navmesh");
	GuiData_Trrn * dataTrrn = (GuiData_Trrn *)FindData("terrain");

	if(!data||!dataTrrn)
		return TRUE;

	AssetSystemState *ss = data->pES->GetAS()->GetSS();
	IPhysWorld * phyWorld = NULL;
	if(ss)
		phyWorld = ss->worldPhys;

	if(!phyWorld)
		return TRUE;
		
	HitProbe rayHit;
	i_math::vector3df vHit;

	IRenderPort * rp = GetRP();
	rp->CalcHitProbe(x,y,rayHit);

	if (phyWorld->RayHitTest(rayHit,/*PhysCollideLayor::*/CldLayor_StaticTest,vHit,NULL))
	{
		data->editParams.bSel=TRUE;
		data->editParams.centerSel = vHit;
	}
	
	return TRUE;
}

BOOL CGuiAgent_NavMeshOp::OnRButtonClick(int x,int y,DWORD flag)
{	
	GuiData_NavMesh * data = (GuiData_NavMesh *)FindData("navmesh");
	if(!data)
		return TRUE;
	
	_AddMenuSep();
	
	_AddMenu("Build NavMesh",ID_AGENT_NAVMESH_CREATE);
	
	return TRUE;	
}

void CGuiAgent_NavMeshOp::_GetRcBlock(INavMeshEditor * editor,i_math::recti &rc)
{
	GuiData_NavMesh * data = (GuiData_NavMesh *)FindData("navmesh");

	if(editor)
	{
		int len = (int)editor->GetMapBlockLen();

		float fx = (data->editParams.centerSel.x);
		float fy = (data->editParams.centerSel.z);

		int x = int(fx)/len;
		int y = int(fy)/len;
		if(fx<0) x -= 1;
		if(fy<0) y -= 1;
		
		int w = data->editParams.wBlock;

		rc.UpperLeftCorner.set(x,y);
		rc.LowerRightCorner.set(x+w,y+w);	
	}
}

BOOL CGuiAgent_NavMeshOp::OnCommand(DWORD idCmd)
{
	INavMeshEditor * editor = NULL;
	INavService * service = NULL;
	GuiData_NavMesh * data = (GuiData_NavMesh *)FindData("navmesh");
	if(data)
	{
		editor =  data->GetEditor();
		service = data->GetNavService();
	}

	if(!editor)
		return TRUE;
	

	switch(idCmd)
	{
		case ID_AGENT_NAVMESH_CREATE:
		{
			if (data->editParams.bSel)
			{
				i_math::recti rc;
				_GetRcBlock(editor,rc);
				editor->Build(data->pES,rc);
				editor->Save();
				break;
			}
		}
		default: 
			break;
	}
	
	return TRUE;
}


void CGuiAgent_NavMeshOp::OnAttachView(CGeView *view,DWORD iLevel)
{
	CGuiAgent::OnAttachView(view,iLevel);
	
}

void CGuiAgent_NavMeshOp::OnDetachView(CGeView *view,DWORD iLevel)
{

	CGuiAgent::OnDetachView(view,iLevel);
}

static void DrawNavmeshTris(IRenderPort *rp,i_math::vector3df *tris,DWORD nTris,BYTE area)
{
	if (nTris<=0)
		return;
	DWORD col;
	switch(area)
	{
		case 63:
			col=0x2200ff00;
			break;
		case 1:
			col=0x22ff00ff;
			break;
		case 2:
			col=0x22ffff00;
			break;
		default:
			col=0x22ffffff;
			break;
	}
	rp->Triangles(tris,nTris,col);
}

BOOL CGuiAgent_NavMeshOp::OnDraw()
{
	IRenderPort * rp = GetRP();

	INavMeshEditor * editor = NULL;
	GuiData_NavMesh * data = (GuiData_NavMesh *)FindData("navmesh");
	if(data)
		editor =  data->GetEditor();
	
	if(!editor)
		return TRUE;
	
	i_math::recti rc;
	_GetRcBlock(editor,rc);

	float len = editor->GetMapBlockLen();

	i_math::aabbox3df aabb;
	aabb.MinEdge.set(rc.Left()*len,data->editParams.centerSel.y,rc.Top()*len);
	aabb.MaxEdge.set(rc.Right()*len,data->editParams.centerSel.y + 20.0f,rc.Bottom()*len);
	
	//Draw Selected Areas
	DrawOBB(rp,i_math::matrix43f(),aabb,0xff00ffff);


	HMapObj hObjSel=NULL;
	if (TRUE)
	{
		i_math::recti rc2;
		rc2=rc;
		rc2*=(int)len;
		rc2.inflate(-2,-2,-2,-2);
		DWORD c;
		HMapObj *sels=editor->Enum(rc2,c);
		if (c>0)
			hObjSel=sels[0];
	}

	NavMeshBuildResult resultBuild;
	editor->GetBuildResult(hObjSel,&resultBuild);

	//Draw Detail Mesh
	if(resultBuild.nTris>0)
	{
		i_math::vector3df *tris=NULL;
		DWORD nTris=0;

		BYTE area=0xff;

		for (int i=0;i<resultBuild.nTris;i++)
		{
			if (resultBuild.areasTris[i]!=area)
			{
				DrawNavmeshTris(rp,tris,nTris,area);
				nTris=1;
				tris=&resultBuild.vtxsTris[i*3];
				area=resultBuild.areasTris[i];
			}
			else
				nTris+=1;
		}
		DrawNavmeshTris(rp,tris,nTris,area);
	}
	
	//Inner Edges
	if(resultBuild.nInnerEdge>0)
		rp->Lines(resultBuild.vtxsInnerEdge,resultBuild.nInnerEdge,0xff00ffff);
	
	//Outer Edges
	if(resultBuild.nOuterEdge>0)
		rp->Lines(resultBuild.vtxsOuterEdge,resultBuild.nOuterEdge,0xffff0000);

	//Other 
	if(TRUE)
	{
		char temp[255];
		sprintf(temp,"NavMesh[ %d , %d ]: %.2f kb ",resultBuild.rx,resultBuild.ry,resultBuild.szKB);
		DrawFontArg arg;
		arg.SetLocation(100,100);
		rp->DrawText(temp,arg);
	}
	

	return TRUE;
}









