#include "stdh.h"
#include ".\guiagent_treeadd.h"
#include "RenderSystem/IRenderPort.h"

#include "WorldSystem/ITrrn.h"
#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IStdRes.h"
#include "GuiData_forest.h"
#include "GuiData.h"

#include "GuiActor_Forest.h"

#include "pin/pin.h"

#include "AgentCmdID.h"

#include "MapObjUtil.h"

CGuiAgent_treeadd::CGuiAgent_treeadd(void)
{
	 _drawer = NULL;
}

CGuiAgent_treeadd::~CGuiAgent_treeadd(void)
{ 
	SAFE_RELEASE(_drawer); 
}
BOOL CGuiAgent_treeadd::OnMouseMove(int x,int y,DWORD flag)
{
	GuiData_Forest * data = static_cast<GuiData_Forest *>(FindData("forest"));
	
	IBrushLib * pLib = NULL;
	IForestEditor *editor = data->GetEditor();
	if(editor)
		pLib = editor->GetSptLib();

	IRenderPort * rp = GetRP();
	
	const IBrush * br = pLib->Get(data->info.refModel);
	if((!pLib)||!pLib->ObtainRes(br))
		return TRUE;	
	
	if(_HitTest(x,y)){
		_Redraw();
	}
	
	return TRUE;	
}

BOOL CGuiAgent_treeadd::_HitTest(int x,int y)
{
	GuiData_Forest * data = static_cast<GuiData_Forest *>(FindData("forest"));
	if(!data)
		return FALSE;

	IRenderPort * rp = GetRP();

	i_math::vector3df intersectPos;
	HitProbe prob;
	rp->CalcHitProbe(x,y,prob);
	GuiData_Trrn *trrn = (GuiData_Trrn *)(FindData("terrain"));
	assert(trrn);

	ITrrnMapEditor * ptrrnEditor = trrn->GetTrrnMapEditor();
	assert(ptrrnEditor);
	if(ptrrnEditor->GetHitPos(prob,TRUE,intersectPos))
	{
		_location = intersectPos;
		data->info.pos = intersectPos;
		return TRUE;
	}

	return FALSE;
}

BOOL CGuiAgent_treeadd::OnLButtonDown(int x,int y,DWORD flag)
{	
	GuiData_Forest *data = static_cast<GuiData_Forest *>(FindData("forest"));
	IBrushLib * pLib = NULL;
	IForestEditor * editor = NULL;
	if(data) 
		editor = data->GetEditor();

	if(editor&&data->bOnAdd)
		pLib = editor->GetSptLib();
	
	const IBrush * br = NULL;
	if(pLib)
		br = pLib->Get(data->info.refModel);
	
	if(br&&pLib->ObtainRes(br)&&_HitTest(x,y)){

		HMapObj hObj = editor->AddTree(data->info);
		
		CModManager * mgr = _GetModMgr();
		if(mgr)
		{
			CommitMapObjMod(mgr,GetView(),hObj,editor);
			data->Reseed();
		}
		else{
			editor->Save();
		}
	}
	
	return TRUE;
}

BOOL CGuiAgent_treeadd::OnRButtonClick(int x,int y,DWORD flag)
{	
	GuiData_Forest *data = static_cast<GuiData_Forest *>(FindData("forest"));

	if(data)
	{
		if(data->bOnAdd){
			data->bOnAdd = FALSE;
			return FALSE;
		}
		else{
			_AddMenuSep();
			_AddMenu("新建",ID_AGENT_FOREST_CREATENEW);
		}
	}
	return TRUE;	
}
BOOL CGuiAgent_treeadd::OnCommand(DWORD idCmd)
{
	GuiData_Forest *data = static_cast<GuiData_Forest *>(FindData("forest"));
	if(!data)
		return TRUE;

	if(idCmd==ID_AGENT_FOREST_CREATENEW){
		data->bOnAdd = TRUE;
		data->Reseed();
	}

	return TRUE;
}
BOOL CGuiAgent_treeadd::OnKeyDown(char c,DWORD flag)
{
	if(c=='Q')
	{
		GuiData_Forest *data = static_cast<GuiData_Forest *>(FindData("forest"));
		if(data)
			data->bOnAdd = FALSE;
		_Redraw();
	}
	return TRUE;
}

BOOL CGuiAgent_treeadd::OnDraw()
{
	IRenderPort * rp = GetRP();	
	GuiData_Forest *data  = static_cast<GuiData_Forest *>(FindData("forest"));
	IForestEditor * editor = NULL;
	if(data)
		editor = data->GetEditor();

	if(!editor||!data->bOnAdd)
		return TRUE;

	if(data->bOnAdd)
	{
		IBrushLib * pSptLib = editor->GetSptLib();
		if(!pSptLib)
			return TRUE;
		
		const IBrush * br = pSptLib->Get(data->info.refModel);
		ISpt * pSpt = (ISpt *)pSptLib->ObtainRes(br);
		if(!_drawer)
			_drawer = g_ssGuiLib.pWS->CreateSptDrawer();
		
		if(_drawer){
			SptDrawParam param;
			param.pos = data->info.pos;
			param.scale = data->info.scale;
			param.rotY = data->info.rotY;
			_drawer->SetRP(rp);
			_drawer->Draw(pSpt,&param);
		}
	}

	return TRUE;
}









